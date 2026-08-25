#include "encoder.h"
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>

void encoder_init(encoder_t *enc, struct encoder_ID ID) {
	enc->ID         = ID;
	enc->initButton = false;
	enc->initValue  = false;
	enc->blink      = false;
	switch (ID.row) {
	case 0:
		enc->color.B = 255;
		enc->color.G = 255;
		enc->color.R = 255;
		break;
	case 1:
		enc->color.B = 255;
		enc->color.G = 255;
		enc->color.R = 255;
		break;
	case 2:
		enc->color.B = 255;
		enc->color.G = 255;
		enc->color.R = 255;
		break;
	case 3:
		enc->color.B = 255;
		enc->color.G = 255;
		enc->color.R = 255;
		break;
	}

	enc->last_change = from_us_since_boot(-1LL);
}

int encoder_push_button(
    encoder_t *enc, bool button, absolute_time_t now, encoder_event_t *event
) {
	if (enc->initButton == false) {
		enc->initButton = true;
		enc->button     = button;
		return 0;
	}

	if (button == false) {
		enc->last_change = now;
	}

	if (enc->button == button) {
		return 0;
	}
	enc->button    = button;
	event->ID      = enc->ID;
	event->ID.type = BUTTON;
	event->button  = !button;
	return 1;
}

#define ABS(a) ((a) < 0 ? (-(a)) : (a))

int encoder_push_fader(
    encoder_t *enc, uint8_t value, absolute_time_t now, encoder_event_t *event
) {
	if (enc->ID.type != FADER) {
		return 0;
	}

	if (enc->initValue == false) {
		enc->initValue = true;
		enc->value     = value;
		return 0;
	}

	if (ABS(enc->value - value) < 2) {
		return 0;
	}

	enc->last_change = now;

	enc->value   = value;
	event->ID    = enc->ID;
	event->fader = value;
	return 1;
}

static inline int8_t diff_4bits(uint8_t old, uint8_t new) {
	uint8_t diff = (int8_t)(new - old) & 0x0f;
	return (int8_t)(((int8_t)diff ^ 0x08) - 0x08);
}

int encoder_push_knob(
    encoder_t *enc, uint8_t value, absolute_time_t now, encoder_event_t *event
) {
	if (enc->ID.type != KNOB) {
		return 0;
	}

	if (enc->initValue == false) {
		enc->initValue = true;
		enc->value     = value;
		return 0;
	}

	if (enc->value == value) {
		return 0;
	}

	enc->last_change = now;

	event->ID    = enc->ID;
	event->delta = diff_4bits(enc->value, value);
	enc->value   = value;
	return 1;
}
