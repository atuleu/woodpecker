#pragma once

#include <hardware/pio.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SECTION_RX_SIZE 16
#define SECTION_RX_MASK (SECTION_RX_SIZE - 1)

_Static_assert(
    SECTION_RX_SIZE > 0 && (SECTION_RX_SIZE & SECTION_RX_MASK) == 0,
    "SECTION_RX_SIZE should be a power of two"
);

struct __attribute__((packed)) section_frame {
	uint8_t  Type : 1;
	uint8_t  ID : 2;
	uint8_t  SequenceID : 3;
	uint16_t Buttons : 10;

	union {
		uint8_t Fader[5];

		struct __attribute__((packed)) {
			// Absolute encoder state is keeped. We do not expect to see
			// more than 16increment in a single ms(it is not event
			// possible). Even with frame loss, getting 1 increment so
			// keeping a 16bit index of the internal position is robust and
			// fine.
			uint8_t A : 4; // state should be absolute.
			uint8_t B : 4; // state should be absolute.
		} Encoder[5];
	};
};
typedef struct section_frame section_frame_t;

enum section_frame_type {
	FRAME_ENCODER = 0,
	FRAME_FADER   = 1,
};
typedef enum section_frame_type section_frame_type_e;

struct __attribute__((packed)) section_packet {
	uint8_t         header;
	section_frame_t frame;
	uint8_t         crc;
};

typedef struct section_packet section_packet_t;

struct section_rx_stats {
	size_t received, locked_errors, framing_errors, crc_errors;
};
typedef struct section_rx_stats section_rx_stats_t;

struct section_rx {
	uint8_t         buffer[SECTION_RX_SIZE * sizeof(section_packet_t)];
	uint32_t        head, tail;
	int             pin;
	PIO             pio;
	int             sm;
	int             offset;
	int             dma;
	size_t          locked_errors, framing_errors, crc_errors;
	absolute_time_t deadline;
};
typedef struct section_rx section_rx_t;

// init an UART_rx on given pin
int  section_rx_init(section_rx_t *rx, int pin, int baudrate);
// deinit UART_rx
void section_rx_deinit(section_rx_t *rx);

// Returns the number of available packet
size_t section_rx_available(section_rx_t *rx);

// Copy as many available packet on buffer. return the number of copied
// packed. Returns the number of packet written (0 or 1).
int section_rx_get(section_rx_t *rx, section_frame_t *frame);

void section_rx_get_stats(section_rx_t *rx, section_rx_stats_t *stats);

// Checks if an UART is blocked (not sufficient character received before an
// internal deadline, and resets it. return true in that case. If the UART
// is fien, returns false.
bool section_rx_check_and_unblock(section_rx_t *rx);

#ifdef __cplusplus
}
#endif
