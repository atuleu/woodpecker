#include <hardware/irq.h>
#include <hardware/platform_defs.h>
#include <hardware/timer.h>
#include <pico/platform/sections.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <hardware/dma.h>
#include <hardware/pio.h>
#include <hardware/sync.h>
#include <pico/error.h>

#include "dma_shared_irq.h"
#include "uart_rx.pio.h"
#include "uart_rx_pio.h"

#define UART_MASK      (UART_SIZE - 1)
#define UART_HALF_SIZE (UART_SIZE / 2)
_Static_assert(
    (UART_SIZE & UART_MASK) == 0, "UART_SIZE must be a power of two"
);

bool pio_claim_free_sm(PIO *pio, int *sm) {
	*sm = pio_claim_unused_sm(pio0, false);
	if (*sm >= 0) {
		*pio = pio0;
		return true;
	}
	*sm = pio_claim_unused_sm(pio1, false);
	if (*sm >= 0) {
		*pio = pio1;
		return true;
	}
#if NUM_PIOS > 2
	*offset = pio_claim_unused_sm(pio2, false);
	if (*offset >= 0) {
		*pio = pio2;
		return true;
	}
#endif
	*pio = NULL;
	return false;
}

static int program_offsets[NUM_PIOS] = {-1, -1};
static UART_Rx_t *contexts[NUM_PIOS * NUM_PIO_STATE_MACHINES] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

void uart_rx_irq_handler(UART_Rx_t *uart, int channel) {
	uart->tail += 1;
}

inline static void uart_rx_pio_irq_handler(UART_Rx_t *uart) {
	dma_channel_set_write_addr(
	    uart->dma,
	    &uart->buffer[(uart->tail & UART_MASK) * UART_PACKET_SIZE],
	    false
	);
	dma_channel_set_transfer_count(uart->dma, UART_PACKET_SIZE, 9);
}

inline static void uart_pioN_irq_handler(PIO pio) {
	uint32_t ints = pio->ints0;
	for (size_t i = 0; i < 4; ++i) {
		UART_Rx_t *uart = contexts[i + PIO_NUM(pio) * NUM_PIO_STATE_MACHINES];
		if (ints & (1u << (pis_interrupt0 + i)) && uart != NULL) {
			uart_rx_pio_irq_handler(uart);
			pio_interrupt_clear(pio, pis_interrupt0 + i);
		}
	}
}

static void __isr __not_in_flash_func(uart_pio0_irq_handler)(void) {
	uart_pioN_irq_handler(pio0);
	uart_pioN_irq_handler(pio1);
}

int UART_Rx_init(UART_Rx_t *uart, int pin, int baudrate) {
	uart->head = 0;
	uart->pin  = pin;
	if (pio_claim_free_sm(&uart->pio, &uart->sm) == false) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}
	int pio_num = PIO_NUM(uart->pio);
	if (program_offsets[pio_num] == 0xffffffff) {
		if (pio_can_add_program(uart->pio, &uart_rx_program) == false) {
			pio_sm_unclaim(uart->pio, uart->sm);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		program_offsets[pio_num] = pio_add_program(uart->pio, &uart_rx_program);
	}
	uart->offset = program_offsets[pio_num];

	uart->dma = dma_claim_unused_channel(true);
	if (uart->dma < 0) {
		pio_sm_unclaim(uart->pio, uart->sm);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	register_dma_channel_handler(
	    DMA_IRQ_0,
	    uart->dma,
	    (dma_channel_irq_handler_fn)uart_rx_irq_handler,
	    uart
	);

	dma_channel_config_t config = dma_channel_get_default_config(uart->dma);
	channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
	channel_config_set_read_increment(&config, false);
	channel_config_set_write_increment(&config, true);
	channel_config_set_dreq(&config, PIO_DREQ_NUM(uart->pio, uart->sm, false));
	dma_channel_set_config(uart->dma, &config, false);
	dma_channel_set_read_addr(
	    uart->dma,
	    (io_rw_8 *)&uart->pio->rxf[uart->sm] + 3,
	    false
	);

	contexts[PIO_NUM(uart->pio) * NUM_PIO_STATE_MACHINES + uart->sm] = uart;
	irq_set_exclusive_handler(
	    pio_get_irq_num(uart->pio, 0),
	    uart_pio0_irq_handler
	);
	pio_set_irqn_source_enabled(uart->pio, 0, pis_interrupt0 + uart->sm, true);
	irq_set_enabled(pio_get_irq_num(uart->pio, 0), true);

	uart_rx_program_init(uart->pio, uart->sm, uart->offset, pin, baudrate);
	return PICO_OK;
}

void UART_Rx_deinit(UART_Rx_t *uart) {

	pio_sm_set_enabled(uart->pio, uart->sm, false);
	pio_sm_unclaim(uart->pio, uart->sm);

	unregister_dma_channel_handler(DMA_IRQ_1, uart->sm);
	dma_channel_abort(uart->sm);
	dma_channel_unclaim(uart->sm);
}

size_t UART_Rx_available(UART_Rx_t *uart) {
	uint32_t saved = save_and_disable_interrupts();

	size_t res = uart->tail - uart->head;
	restore_interrupts(saved);

	return res;
}

uint8_t UART_Rx_getc(UART_Rx_t *uart) {
	if (UART_Rx_available(uart) == 0) {
		return 0xff;
	}
	uint8_t res = uart->buffer[uart->head];
	uart->head  = (uart->head + 1) & UART_MASK;
	return res;
}

uint8_t UART_Rx_unsafe_getc(UART_Rx_t *uart) {
	uint8_t res = uart->buffer[uart->head];
	uart->head  = (uart->head + 1) & UART_MASK;
	return res;
}
