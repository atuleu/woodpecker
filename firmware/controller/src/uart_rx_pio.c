#include "uart_rx_pio.h"

#include <hardware/irq.h>
#include <hardware/pio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <hardware/dma.h>
#include <pico/time.h>

#include "atomic.h"
#include "dma_shared_irq.h"
#include "uart_rx.pio.h"

static inline pio_interrupt_source_t
pio_get_relative_sm_interrupt_source(int rel_int, uint sm) {
	return ((pio_interrupt_source_t)(pis_interrupt0 + (rel_int + sm) % 4));
}

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

void pio_sm_restart_at_offset(PIO pio, int sm, int offset) {
	pio_sm_set_enabled(pio, sm, false);
	const uint32_t fdebug_sm_mask =
	    (1u << PIO_FDEBUG_TXOVER_LSB) | (1u << PIO_FDEBUG_RXUNDER_LSB) |
	    (1u << PIO_FDEBUG_TXSTALL_LSB) | (1u << PIO_FDEBUG_RXSTALL_LSB);
	pio->fdebug = fdebug_sm_mask << sm;

	pio_sm_restart(pio, sm);
	pio_sm_clkdiv_restart(pio, sm);
	pio_sm_exec(pio, sm, pio_encode_jmp(offset));
	pio_sm_set_enabled(pio, sm, true);
}

#if NUM_PIOS == 2 && NUM_PIO_STATE_MACHINES == 4
static int        program_offsets[NUM_PIOS] = {0xffffffff, 0xffffffff};
static UART_Rx_t *contexts[NUM_PIOS * NUM_PIO_STATE_MACHINES] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#endif

void uart_rx_dma_irq_handler(UART_Rx_t *uart, int channel) {
	uart->tail += 1;
	uart->deadline = from_us_since_boot(-1);
	pio_sm_restart_at_offset(uart->pio, uart->sm, uart->offset);
}

inline static void uart_rx_pio_irq_handler(UART_Rx_t *uart) {
	dma_channel_set_write_addr(
	    uart->dma,
	    &uart->buffer[(uart->tail & UART_MASK) * UART_PACKET_SIZE],
	    false
	);
	dma_channel_set_transfer_count(
	    uart->dma,
	    dma_encode_transfer_count(UART_PACKET_SIZE),
	    true
	);
	uart->deadline = make_timeout_time_ms(5);
}

inline static void _uart_pio_irq0_handler(PIO pio) {
	uint32_t ints = pio->ints0;
	for (size_t i = 0; i < 4; ++i) {
		UART_Rx_t *uart = contexts[i + PIO_NUM(pio) * NUM_PIO_STATE_MACHINES];
		if (ints & (1u << (pis_interrupt0 + i))) {
			if (uart != NULL) {
				uart_rx_pio_irq_handler(uart);
			}
			pio_interrupt_clear(pio, i);
		}
	}
}

static void __isr __not_in_flash_func(uart_pio0_irq0_handler)(void) {
	_uart_pio_irq0_handler(pio0);
}

static void __isr __not_in_flash_func(uart_pio1_irq0_handler)(void) {
	_uart_pio_irq0_handler(pio1);
}

int UART_Rx_init(UART_Rx_t *uart, int pin, int baudrate) {
	uart->head      = 0;
	uart->tail      = 0;
	uart->tx_errors = 0;
	uart->pin       = pin;
	uart->deadline  = from_us_since_boot(-1);

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

	register_dma_channel_handler(
	    DMA_IRQ_0,
	    uart->dma,
	    (dma_channel_irq_handler_fn)uart_rx_dma_irq_handler,
	    uart
	);

	ATOMIC_CORE_BLOCK() {
		contexts[PIO_NUM(uart->pio) * NUM_PIO_STATE_MACHINES + uart->sm] = uart;
	};
	if (uart->pio == pio0) {
		irq_set_exclusive_handler(
		    pio_get_irq_num(uart->pio, 0),
		    uart_pio0_irq0_handler
		);
	}
	if (uart->pio == pio1) {
		irq_set_exclusive_handler(
		    pio_get_irq_num(uart->pio, 0),
		    uart_pio1_irq0_handler
		);
	}
	// irq_set_priority(pio_get_irq_num(uart->pio, 0),
	// PICO_HIGHEST_IRQ_PRIORITY);
	pio_set_irqn_source_enabled(
	    uart->pio,
	    0,
	    pio_get_relative_sm_interrupt_source(uart_rx_IRQ_READY, uart->sm),
	    true
	);

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
	size_t res;
	ATOMIC_CORE_BLOCK() {
		res = uart->tail - uart->head;
	}
	return res;
}

int UART_Rx_get(UART_Rx_t *uart, uint8_t *buffer, size_t len) {
	if (len < UART_PACKET_SIZE) {
		return PICO_ERROR_BUFFER_TOO_SMALL;
	}
	int res;
	ATOMIC_CORE_BLOCK() {
		uint32_t available = uart->tail - uart->head;

		res = MIN(len / UART_PACKET_SIZE, available);
		for (size_t i = 0; i < res; ++i) {
			memcpy(
			    &buffer[i * UART_PACKET_SIZE],
			    &uart->buffer[(uart->head++ & UART_MASK) * UART_PACKET_SIZE],
			    UART_PACKET_SIZE
			);
		}
	}
	return res;
}

bool UART_Rx_check_and_unblock(UART_Rx_t *uart) {
	ATOMIC_CORE_BLOCK() {
		if (absolute_time_diff_us(uart->deadline, get_absolute_time()) < 0) {

			return false;
		}

		pio_sm_set_enabled(uart->pio, uart->sm, false);
		dma_channel_abort(uart->dma);

		// clear mis-triggerred interupt.
		dma_channel_acknowledge_irq0(uart->dma);
		pio_interrupt_clear(
		    uart->pio,
		    pio_get_relative_sm_interrupt_source(uart_rx_IRQ_READY, uart->sm)
		);
		pio_sm_restart_at_offset(uart->pio, uart->sm, uart->offset);

		uart->tx_errors += 1;
	}
	return true;
}
