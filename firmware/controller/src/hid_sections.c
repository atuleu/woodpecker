#include "hid_sections.h"
#include "uart_rx_pio.h"
#include <hardware/irq.h>
#include <pico/error.h>
#include <pico/platform/sections.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

#define BAUDRATE     230400
#define NUM_SECTIONS 1

struct __attribute__((packed)) section_frame {
	uint8_t  Type : 1;
	uint8_t  ID : 2;
	uint8_t  SequenceID : 3;
	uint16_t Buttons : 10;

	union {
		uint8_t Fader[5];

		struct __attribute__((packed)) {
			// Absolute encoder state is keeped. We do not expect to see more
			// than 16increment in a single ms(it is not event possible). Even
			// with frame loss, getting 1 increment so keeping a 16bit index of
			// the internal position is robust and fine.
			uint8_t A : 4; // state should be absolute.
			uint8_t B : 4; // state should be absolute.
		} Encoder[5];
	};
};

typedef struct section_frame section_frame_t;

_Static_assert(sizeof(section_frame_t) == 7, "Size of frame should be 7");

struct hid_section {
	UART_Rx_t uart_rx;

	size_t disp;
	size_t crc_errors, framing_errors;
};
typedef struct hid_section hid_section_t;

int hid_section_init(hid_section_t *section, int pin, int baudrate) {
	int err = UART_Rx_init(&section->uart_rx, pin, baudrate);
	if (err != PICO_OK) {
		return err;
	}
	section->disp           = -1;
	section->crc_errors     = 0;
	section->framing_errors = 0;
	return PICO_OK;
}

void hid_section_deinit(hid_section_t *section) {
	UART_Rx_deinit(&section->uart_rx);
}

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

struct __attribute__((packed)) section_packet {
	uint8_t         header;
	section_frame_t frame;
	uint8_t         crc;
} packet;

typedef struct section_packet section_packet_t;
_Static_assert(
    sizeof(section_packet_t) == UART_PACKET_SIZE, "Invalid UART configuration"
);

inline static void
hid_section_handle_frame(hid_section_t *section, section_frame_t *frame) {
	if (section->disp++ % 1000 != 0) {
		return;
	}
	if (frame->Type == 0) {
		printf(
		    "Got frame Type=ENCODER ID=%d SeqID=%d Buttons=%02X "
		    "Encoders=[%d %d %d %d %d %d %d %d %d %d]\n",
		    frame->ID,
		    frame->SequenceID,
		    frame->Buttons,
		    frame->Encoder[0].A,
		    frame->Encoder[0].B,
		    frame->Encoder[1].A,
		    frame->Encoder[1].B,
		    frame->Encoder[2].A,
		    frame->Encoder[2].B,
		    frame->Encoder[3].A,
		    frame->Encoder[3].B,
		    frame->Encoder[4].A,
		    frame->Encoder[4].B
		);
	} else {
		printf(
		    "Got frame Type=FADER ID=%d SeqID=%d Buttons=%02X "
		    "Faders=[%d %d %d %d %d]\n",
		    frame->ID,
		    frame->SequenceID,
		    frame->Buttons,
		    frame->Fader[0],
		    frame->Fader[1],
		    frame->Fader[2],
		    frame->Fader[3],
		    frame->Fader[4]
		);
	}
}

static void hid_section_work(hid_section_t *section) {
	union {
		section_packet_t as_packet;
		uint8_t          as_buffer[UART_PACKET_SIZE];
	} p;

	bool bad = false;
	while (true) {
		int res = UART_Rx_get(
		    &section->uart_rx,
		    p.as_buffer,
		    sizeof(section_packet_t)
		);
		if (res <= 0) {
			break;
		}

		if (p.as_packet.header != 0x55) {
			section->framing_errors += 1;
			bad = true;
			break;
		}

		for (size_t i = 0; i < sizeof(section_frame_t); ++i) {
			p.as_packet.crc =
			    crc8_0x31_update(p.as_packet.crc, p.as_buffer[i + 1]);
		}
		if (p.as_packet.crc != 0) {
			section->crc_errors += 1;
			bad = true;
			break;
		}

		hid_section_handle_frame(section, &p.as_packet.frame);
	}
	if (bad == false) {
		return;
	}
	if (section->disp++ % 1000 == 0) {
		printf(
		    "Got invalid frame: stats: rx: %d rx_error: %d frame_error:%d "
		    "crc_error:%d\n",
		    section->uart_rx.tail,
		    section->uart_rx.tx_errors,
		    section->framing_errors,
		    section->crc_errors
		);
		printf(
		    "%02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
		    p.as_buffer[0],
		    p.as_buffer[1],
		    p.as_buffer[2],
		    p.as_buffer[3],
		    p.as_buffer[4],
		    p.as_buffer[5],
		    p.as_buffer[6],
		    p.as_buffer[7],
		    p.as_buffer[8]
		);
	}
}

struct hid_sections {
	int               irq;
	repeating_timer_t timer;

	struct hid_section sections[NUM_SECTIONS];
};

static struct hid_sections sections = {
    .irq   = -1,
    .timer = {.user_data = NULL},
};

static void __isr __not_in_flash_func(hid_section_irq_handler)(void) {
	for (size_t i = 0; i < NUM_SECTIONS; ++i) {
		hid_section_work(&sections.sections[i]);
	}
	irq_clear(sections.irq);
}

static bool hid_section_repeated_timer(__unused repeating_timer_t *rt) {
	irq_set_pending(sections.irq);
	return true;
}

void _hid_sections_deinit(size_t sections);

int hid_sections_init() {
	if (sections.irq >= 0) {
		return PICO_ERROR_INVALID_STATE;
	}

	int pins[6] = {2, 3, 4, 23, 24, 25};
	for (size_t i = 0; i < NUM_SECTIONS; ++i) {
		int err = hid_section_init(&sections.sections[i], pins[i], BAUDRATE);
		if (err != PICO_OK) {
			_hid_sections_deinit(i);
			return err;
		}
	}

	sections.irq = user_irq_claim_unused(false);
	if (sections.irq < 0) {
		_hid_sections_deinit(NUM_SECTIONS);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}
	irq_set_exclusive_handler(sections.irq, hid_section_irq_handler);

	irq_set_enabled(sections.irq, true);
	// sets the lowest priority for this user IRQ. we need it to be prempted.
	irq_set_priority(sections.irq, PICO_LOWEST_IRQ_PRIORITY);
	if (add_repeating_timer_us(
	        250,
	        hid_section_repeated_timer,
	        &sections,
	        &sections.timer
	    ) == false) {
		_hid_sections_deinit(NUM_SECTIONS);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	return PICO_OK;
}

void hid_sections_deinit() {
	_hid_sections_deinit(NUM_SECTIONS);
}

void _hid_sections_deinit(size_t num_sections) {
	if (sections.irq >= 0) {
		irq_set_enabled(sections.irq, false);
		irq_remove_handler(sections.irq, hid_section_irq_handler);
	}

	if (sections.timer.user_data != NULL) {
		cancel_repeating_timer(&sections.timer);
	}

	for (size_t i = 0; i < num_sections; ++i) {
		hid_section_deinit(&sections.sections[i]);
	}
}
