#pragma once

#include <stdbool.h>
#include <stdint.h>

struct __attribute__((packed)) Frame {
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

_Static_assert(sizeof(struct Frame) == 7, "Frame should be 7 bytes long");

enum FrameType {
	ENCODER = 0,
	FADER   = 1,
};

inline static void Frame_set_button(struct Frame *f, uint8_t idx, bool value) {
	if (value == true) {
		f->Buttons |= ((uint16_t)1 << idx);
	} else {
		f->Buttons &= ~((uint16_t)1 << idx);
	}
}

inline static void
Frame_set_encoder(struct Frame *f, uint8_t idx, uint8_t value) {
	// get the right index.
	if ((idx & 0x01) == 0x00) {
		f->Encoder[idx >> 1].A = value & 0x0f;
	} else {
		f->Encoder[idx >> 1].B = value & 0x0f;
	}
}

inline static int8_t Frame_get_encoder(const struct Frame *f, uint8_t idx) {
	// get value from the right index.
	if ((idx & 0x01) == 0x00) {
		return f->Encoder[idx >> 1].A;
	} else {
		return f->Encoder[idx >> 1].B;
	}
}
