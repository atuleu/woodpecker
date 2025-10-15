#pragma once

#include <pico/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__((packed)) color {
	uint8_t R, G, B;
};
typedef struct color color_t;

enum encoder_type {
	BUTTON = 0,
	FADER  = 1,
	KNOB   = 2,
};

struct __attribute__((packed)) encoder_ID {
	enum encoder_type type : 2;
	uint8_t           row : 2;
	uint8_t           col : 4;
};

struct encoder {
	struct encoder_ID ID;
	color_t           color;
	absolute_time_t   last_change;
	bool              button;
	bool              initButton;
	bool              initValue;
	uint8_t           value;
};
typedef struct encoder encoder_t;

struct encoder_event {
	struct encoder_ID ID;

	union {
		uint8_t fader;
		bool    button;
		int8_t  delta;
	};
};
typedef struct encoder_event encoder_event_t;

void encoder_init(encoder_t *enc, struct encoder_ID ID);

int encoder_push_button(encoder_t *enc, bool button, encoder_event_t *event);
int encoder_push_fader(encoder_t *enc, uint8_t value, encoder_event_t *event);
int encoder_push_knob(encoder_t *enc, uint8_t value, encoder_event_t *event);

#ifdef __cplusplus
}
#endif
