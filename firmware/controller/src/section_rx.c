#include "section_rx.h"

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
static section_rx_t *contexts[NUM_PIOS * NUM_PIO_STATE_MACHINES] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#endif

static inline uint8_t crc8_0x31_update(uint8_t crc, uint8_t data) {
	const static uint8_t crc8_0x31[256] = {
	    0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 0xB9, 0x88, 0xDB, 0xEA,
	    0x7D, 0x4C, 0x1F, 0x2E, 0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
	    0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D, 0x86, 0xB7, 0xE4, 0xD5,
	    0x42, 0x73, 0x20, 0x11, 0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
	    0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F,
	    0xB8, 0x89, 0xDA, 0xEB, 0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
	    0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13, 0x7E, 0x4F, 0x1C, 0x2D,
	    0xBA, 0x8B, 0xD8, 0xE9, 0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
	    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02, 0x33, 0x60, 0x51,
	    0xC6, 0xF7, 0xA4, 0x95, 0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
	    0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6, 0x7A, 0x4B, 0x18, 0x29,
	    0xBE, 0x8F, 0xDC, 0xED, 0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
	    0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE, 0x80, 0xB1, 0xE2, 0xD3,
	    0x44, 0x75, 0x26, 0x17, 0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
	    0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2, 0xBF, 0x8E, 0xDD, 0xEC,
	    0x7B, 0x4A, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
	    0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD,
	    0x3A, 0x0B, 0x58, 0x69, 0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
	    0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A, 0xC1, 0xF0, 0xA3, 0x92,
	    0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
	    0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A, 0x59, 0x68,
	    0xFF, 0xCE, 0x9D, 0xAC,
	};

	return crc8_0x31[crc ^ data];
}

void uart_rx_dma_irq_handler(section_rx_t *rx, int channel) {
	do {
		uint8_t *b =
		    &rx->buffer
		         [((rx->tail) & SECTION_RX_MASK) * sizeof(section_packet_t)];

		section_packet_t *p = (section_packet_t *)b;

		if (p->header != 0x55) {
			rx->framing_errors += 1;
			break;
		}
		uint8_t crc = 0;
		for (size_t i = 1; i < sizeof(section_packet_t) - 1; ++i) {
			crc = crc8_0x31_update(crc, b[i]);
		}
		if (crc != p->crc) {
			rx->crc_errors += 1;
		}
		rx->tail += 1;
	} while (0);

	rx->deadline = from_us_since_boot(-1);
	pio_sm_restart_at_offset(rx->pio, rx->sm, rx->offset);
}

inline static void uart_rx_pio_irq_handler(section_rx_t *rx) {
	dma_channel_set_write_addr(
	    rx->dma,
	    &rx->buffer[(rx->tail & SECTION_RX_MASK) * sizeof(section_packet_t)],
	    false
	);
	dma_channel_set_transfer_count(
	    rx->dma,
	    dma_encode_transfer_count(sizeof(section_packet_t)),
	    true
	);
	rx->deadline = make_timeout_time_ms(5);
}

inline static void _uart_pio_irq0_handler(PIO pio) {
	uint32_t ints = pio->ints0;
	for (size_t i = 0; i < 4; ++i) {
		section_rx_t *uart =
		    contexts[i + PIO_NUM(pio) * NUM_PIO_STATE_MACHINES];
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

int section_rx_init(section_rx_t *rx, int pin, int baudrate) {
	rx->head           = 0;
	rx->tail           = 0;
	rx->locked_errors  = 0;
	rx->crc_errors     = 0;
	rx->framing_errors = 0;
	rx->pin            = pin;
	rx->deadline       = from_us_since_boot(-1);

	if (pio_claim_free_sm(&rx->pio, &rx->sm) == false) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	int pio_num = PIO_NUM(rx->pio);
	if (program_offsets[pio_num] == 0xffffffff) {
		if (pio_can_add_program(rx->pio, &uart_rx_program) == false) {
			pio_sm_unclaim(rx->pio, rx->sm);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		program_offsets[pio_num] = pio_add_program(rx->pio, &uart_rx_program);
	}
	rx->offset = program_offsets[pio_num];

	rx->dma = dma_claim_unused_channel(true);
	if (rx->dma < 0) {
		pio_sm_unclaim(rx->pio, rx->sm);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	dma_channel_config_t config = dma_channel_get_default_config(rx->dma);
	channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
	channel_config_set_read_increment(&config, false);
	channel_config_set_write_increment(&config, true);
	channel_config_set_dreq(&config, PIO_DREQ_NUM(rx->pio, rx->sm, false));
	dma_channel_set_config(rx->dma, &config, false);
	dma_channel_set_read_addr(
	    rx->dma,
	    (io_rw_8 *)&rx->pio->rxf[rx->sm] + 3,
	    false
	);

	register_dma_channel_handler(
	    DMA_IRQ_0,
	    rx->dma,
	    (dma_channel_irq_handler_fn)uart_rx_dma_irq_handler,
	    rx
	);

	ATOMIC_CORE_BLOCK() {
		contexts[PIO_NUM(rx->pio) * NUM_PIO_STATE_MACHINES + rx->sm] = rx;
	};
	if (rx->pio == pio0) {
		irq_set_exclusive_handler(
		    pio_get_irq_num(rx->pio, 0),
		    uart_pio0_irq0_handler
		);
	}
	if (rx->pio == pio1) {
		irq_set_exclusive_handler(
		    pio_get_irq_num(rx->pio, 0),
		    uart_pio1_irq0_handler
		);
	}
	// irq_set_priority(pio_get_irq_num(uart->pio, 0),
	// PICO_HIGHEST_IRQ_PRIORITY);
	pio_set_irqn_source_enabled(
	    rx->pio,
	    0,
	    pio_get_relative_sm_interrupt_source(uart_rx_IRQ_READY, rx->sm),
	    true
	);

	irq_set_enabled(pio_get_irq_num(rx->pio, 0), true);

	uart_rx_program_init(rx->pio, rx->sm, rx->offset, pin, baudrate);
	return PICO_OK;
}

void section_rx_deinit(section_rx_t *rx) {

	pio_sm_set_enabled(rx->pio, rx->sm, false);
	pio_sm_unclaim(rx->pio, rx->sm);

	unregister_dma_channel_handler(DMA_IRQ_1, rx->sm);
	dma_channel_abort(rx->sm);
	dma_channel_unclaim(rx->sm);
}

size_t UART_Rx_available(section_rx_t *rx) {
	size_t res;
	ATOMIC_CORE_BLOCK() {
		res = rx->tail - rx->head;
	}
	return res;
}

int section_rx_get(section_rx_t *rx, section_frame_t *frame) {
	int res;
	ATOMIC_CORE_BLOCK() {
		if (rx->tail == rx->head) {
			return 0;
		}
		memcpy(
		    frame,
		    &rx->buffer
		         [(rx->head++ & SECTION_RX_MASK) * sizeof(section_packet_t) +
		          1],
		    sizeof(section_frame_t)
		);
	}

	return 1;
}

bool section_rx_check_and_unblock(section_rx_t *rx) {
	ATOMIC_CORE_BLOCK() {
		if (absolute_time_diff_us(rx->deadline, get_absolute_time()) < 0) {

			return false;
		}

		pio_sm_set_enabled(rx->pio, rx->sm, false);
		dma_channel_abort(rx->dma);

		// clear mis-triggerred interupt.
		dma_channel_acknowledge_irq0(rx->dma);
		pio_interrupt_clear(
		    rx->pio,
		    pio_get_relative_sm_interrupt_source(uart_rx_IRQ_READY, rx->sm)
		);
		pio_sm_restart_at_offset(rx->pio, rx->sm, rx->offset);

		rx->locked_errors += 1;
	}
	return true;
}

void section_rx_get_stats(section_rx_t *rx, section_rx_stats_t *stats) {
	ATOMIC_CORE_BLOCK() {
		stats->received       = rx->tail;
		stats->locked_errors  = rx->locked_errors;
		stats->framing_errors = rx->framing_errors;
		stats->crc_errors     = rx->crc_errors;
	}
}
