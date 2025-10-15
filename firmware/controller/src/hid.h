#pragma once

#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HID_NUM_SECTIONS
#define HID_NUM_SECTIONS 1
#endif

#define HID_NUM_ENCODERS (20 * HID_NUM_SECTIONS)

struct __attribute__((packed)) color {
	uint8_t R, G, B;
};
typedef struct color color_t;

enum encoder_type {
	BUTTON,
	FADER,
	KNOB,
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
	uint8_t           value;
};
typedef struct encoder encoder_t;

struct hid_event {
	struct encoder_ID ID;

	union {
		uint8_t fader;
		bool    button;
		int8_t  knob_delta;
	};
};
typedef struct hid_event hid_event_t;
int hid_init();

void hid_deinit();

int hid_get_state(encoder_t *encoders, size_t len);

int hid_pull_event(hid_event_t *event);

#ifdef __cplusplus
}
#endif
