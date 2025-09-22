#pragma once

#include <stdbool.h>
#include <stdint.h>

struct __attribute__((packed)) Frame {
	uint8_t ID : 6;

	uint16_t Buttons : 10;

	union {
		uint8_t Fader[5];

		struct __attribute__((packed)) {
			uint8_t A : 4;
			uint8_t B : 4;
		} Encoder[5];
	};
};

_Static_assert(sizeof(struct Frame) == 7, "Frame should be of size 7");

enum FrameType {
	ENCODER = 0,
	FADER   = 1,
};

inline void Frame_set_ID(struct Frame *f, uint8_t ID, enum FrameType type) {
	f->ID = ((uint8_t)type << 2) | (ID & 0x03);
}

inline void Frame_set_fader(struct Frame *f, uint8_t idx, uint8_t value) {
	f->Fader[idx] = value;
}

inline void Frame_set_button(struct Frame *f, uint8_t idx, bool value) {
	if (value == true) {
		f->Buttons |= (1 << idx);
	} else {
		f->Buttons &= ~(1 << idx);
	}
}

inline uint8_t to_int4_t(int8_t v) {
	if ((v & 0x80) != 0) {
		return 0x08 | (v & 0x07);
	} else {
		return v & 0x07;
	}
}

inline int8_t from_int4_t(uint8_t v) {
	if ((v & 0x8) != 0) {
		return 0x80 | (v & 0x07);
	} else {
		return v & 0x07;
	}
}

inline void Frame_set_encoder(struct Frame *f, uint8_t idx, int8_t value) {

	// get the right index.
	if ((idx & 0x01) == 0x00) {
		f->Encoder[idx >> 1].A = to_int4_t(value);
	} else {
		f->Encoder[idx >> 1].B = to_int4_t(value);
	}
}

int8_t Frame_get_encoder(const struct Frame *f, uint8_t idx) {
	// get value from the right index.
	if ((idx & 0x01) == 0x00) {
		return from_int4_t(f->Encoder[idx >> 1].A);
	} else {
		return from_int4_t(f->Encoder[idx >> 1].B);
	}
}
