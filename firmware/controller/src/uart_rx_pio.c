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

bool pio_claim_free_sm(PIO *pio, int *offset) {
	*offset = pio_claim_unused_sm(pio0, false);
	if (*offset >= 0) {
		*pio = pio0;
		return true;
	}
	*offset = pio_claim_unused_sm(pio1, false);
	if (*offset >= 0) {
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

static int  program_offsets[NUM_PIOS];
static bool init = false;

void uart_rx_irq_handler(UART_Rx_t *uart, int channel) {
	if ((uart->_count % 2 == 0 && uart->head >= UART_HALF_SIZE) ||
	    (uart->_count % 2 == 1 && uart->head < UART_HALF_SIZE)) {
		printf("warning: possible RX overflow");
	}

	dma_channel_set_write_addr(
	    channel,
	    uart->buffer + (++uart->_count % 2) * UART_SIZE / 2,
	    true
	);
}

int UART_Rx_init(UART_Rx_t *uart, int pin, int baudrate) {
	if (!init) {
		init = true;
		memset(program_offsets, 0xff, NUM_PIOS * sizeof(int));
	}
	uart->head   = 0;
	uart->pin    = pin;
	uart->_count = 0;
	if (pio_claim_free_sm(&uart->_pio, &uart->_sm) == false) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}
	int pio_num = PIO_NUM(uart->_pio);
	if (program_offsets[pio_num] == 0xffffffff) {
		if (pio_can_add_program(uart->_pio, &uart_rx_program) == false) {
			pio_sm_unclaim(uart->_pio, uart->_sm);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		program_offsets[pio_num] =
		    pio_add_program(uart->_pio, &uart_rx_program);
	}
	uart->_offset = program_offsets[pio_num];

	uart->_dma = dma_claim_unused_channel(true);
	if (uart->_dma < 0) {
		pio_sm_unclaim(uart->_pio, uart->_sm);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	register_dma_channel_handler(
	    DMA_IRQ_1,
	    uart->_dma,
	    (dma_channel_irq_handler_fn)uart_rx_irq_handler,
	    uart
	);

	dma_channel_config_t config = dma_channel_get_default_config(uart->_dma);
	channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
	channel_config_set_read_increment(&config, false);
	channel_config_set_write_increment(&config, true);
	channel_config_set_dreq(
	    &config,
	    PIO_DREQ_NUM(uart->_pio, uart->_sm, false)
	);

	dma_channel_configure(
	    uart->_dma,
	    &config,
	    uart->buffer,
	    (io_rw_8 *)&uart->_pio->rxf[uart->_sm] + 3,
	    UART_SIZE,
	    true
	);

	uart_rx_program_init(uart->_pio, uart->_sm, uart->_offset, pin, baudrate);
	return PICO_OK;
}

void UART_Rx_deinit(UART_Rx_t *uart) {

	pio_sm_set_enabled(uart->_pio, uart->_sm, false);
	pio_sm_unclaim(uart->_pio, uart->_sm);

	unregister_dma_channel_handler(DMA_IRQ_1, uart->_sm);
	dma_channel_abort(uart->_sm);
	dma_channel_unclaim(uart->_sm);
}

size_t UART_Rx_available(UART_Rx_t *uart) {
	uint32_t saved = save_and_disable_interrupts();

	uint32_t tail = (1 + uart->_count & 0x01) * (UART_SIZE / 2) -
	                dma_hw->ch->transfer_count;
	restore_interrupts(saved);

	if (uart->head <= tail) {
		return tail - uart->head;
	}
	return UART_SIZE + tail - uart->head;
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
