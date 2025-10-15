#include "encoder.h"

void encoder_init(encoder_t *enc, struct encoder_ID ID) {
	enc->ID         = ID;
	enc->initButton = false;
	enc->initValue  = false;
}

int encoder_push_button(encoder_t *enc, bool button, encoder_event_t *event) {
	if (enc->initButton == false) {
		enc->initButton = true;
		enc->button     = button;
		return 0;
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

int encoder_push_fader(encoder_t *enc, uint8_t value, encoder_event_t *event) {
	if (enc->ID.type != FADER) {
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

	enc->value   = value;
	event->ID    = enc->ID;
	event->fader = value;
	return 1;
}

int encoder_push_knob(encoder_t *enc, uint8_t value, encoder_event_t *event) {
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

	event->ID    = enc->ID;
	event->delta = value - enc->value;
	enc->value   = value;
	return 1;
}
