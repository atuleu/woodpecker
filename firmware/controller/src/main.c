#include <hardware/sync.h>
#include <lwip/err.h>
#include <pico/util/queue.h>
#include <stdio.h>

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/error.h>
#include <pico/multicore.h>
#include <pico/platform/common.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/types.h>
#include <string.h>

#include "atomic.h"
#include "encoder.h"
#include "hid.h"
#include "hid_visual.h"
#include "netusb.h"
#include "osc.h"

#define PERIOD_us (20 * 1000)
#define BAUDRATE  400 * 1000
#define WAIT      false

static bool schedule_render = true;

char osc_addresses[3 * 4 * 5 * HID_NUM_SECTIONS * 8];

int init_osc_addresses() {
	encoder_t encoders[HID_NUM_ENCODERS];
	hid_get_state(encoders, HID_NUM_ENCODERS);

	for (size_t i = 0; i < HID_NUM_ENCODERS; ++i) {
		struct encoder_ID ID = encoders[i].ID;

		int n = snprintf(
		    &osc_addresses[(BUTTON * HID_NUM_ENCODERS + i) * 8],
		    8,
		    "/But%d%02d",
		    ID.row + 1,
		    ID.col + 1
		);
		if (n != 7) {
			printf(
			    "[osc]: could not initialize addresses /But%d%02d\n",
			    ID.row + 1,
			    ID.col + 1
			);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		n = snprintf(
		    &osc_addresses[(FADER * HID_NUM_ENCODERS + i) * 8],
		    8,
		    "/Fad%d%02d",
		    ID.row + 1,
		    ID.col + 1
		);
		if (n != 7) {
			printf(
			    "[osc]: could not initialize addresses /Fad%d%02d\n",
			    ID.row + 1,
			    ID.col + 1
			);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		n = snprintf(
		    &osc_addresses[(KNOB * HID_NUM_ENCODERS + i) * 8],
		    8,
		    "/Enc%d%02d",
		    ID.row + 1,
		    ID.col + 1
		);
		if (n != 7) {
			printf(
			    "[osc]: could not initialize addresses /Enc%d%02d\n",
			    ID.row + 1,
			    ID.col + 1
			);
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
	}
	return PICO_OK;
}

const char *get_osc_address(struct encoder_ID ID) {
	return &osc_addresses
	    [(ID.type * HID_NUM_ENCODERS + HID_ENCODER_IDX(ID.row, ID.col)) * 8];
}

void core1_main() {
	multicore_lockout_victim_init();

	ATOMIC_CORE_BLOCK() {
		int err = hid_init();
		if (err != PICO_OK) {
			printf("[hid] Initialization failed: %d\n", err);
			return;
		}
		printf("[hid] Initialized.\n");

		// Initialize addresses before sending event through background task
		// (hence the ATOMIC_CORE_BLOCK);
		err = init_osc_addresses();
		if (err != PICO_OK) {
			printf("[hid] Could not initialize OSC addresses\n");
			return;
		}
	}

	int err = hid_visual_init();
	if (err != PICO_OK) {
		printf("[hid_visual] Initialization failed:%d\n", err);
		return;
	}
	printf("[hid_visual] Initialized.\n");
	while (true) {
		hid_task();
		hid_visual_task();
	}
}

void send_event_to_osc(encoder_event_t *event) {
	OSC_message_t message = {.address = get_osc_address(event->ID)};
	switch (event->ID.type) {
	case BUTTON:
		message.argument.type = event->button ? OSC_TRUE : OSC_FALSE;
		break;
	case FADER:
		message.argument.type         = OSC_INT32;
		message.argument.data.integer = event->fader;
		break;
	case KNOB:
		message.argument.type         = OSC_INT32;
		message.argument.data.integer = event->delta;
		break;
	}

	printf("sending %s %x\n", message.address, event->delta);
	int err;
	ATOMIC_CORE_BLOCK() {
		err = osc_send(&message);
	}
	if (err != ERR_OK) {
		printf(
		    "could not send OSC message: %d address=%s\n",
		    err,
		    get_osc_address(event->ID)
		);
	}
}

int32_t parse_hex_color(const char *str) {
	if (str[0] != '#') {
		return -1;
	}
	char *str_end = NULL;
	int   color   = strtol(str + 1, &str_end, 16);
	if (str_end != str + 7) {
		return -1;
	}
	return color;
}

static inline color_t color_from_int32_t(int32_t c) {
	return (color_t){
	    .R = (c & 0x00ff0000) >> 16,
	    .G = (c & 0x0000ff00) >> 8,
	    .B = (c & 0x000000ff) >> 0,
	};
}

void on_osc_message(void *arg, OSC_message_t *message) {
	(void)arg;

	if (strncmp(message->address, "/Enc", 4) != 0 ||
	    strlen(message->address) != 7) {
		printf("[osc] Invalid address %s\n", message->address);
		return;
	}
	encoder_update_t update;
	switch (message->address[4]) {
	case '1':
	case '2':
	case '3':
	case '4':
		update.ID.row = message->address[4] - '1';
		break;
	default:
		printf("[osc] Invalid encoder number %s\n", &message->address[4]);
		return;
	}
	char *str_end = NULL;
	int   col     = strtol(&message->address[5], &str_end, 10);
	if (str_end != &message->address[7] || col <= 0 || col > 15) {
		printf("[osc] Invalid encoder number %s\n", &message->address[4]);
		return;
	}
	update.ID.col = col - 1;
	int32_t color;
	switch (message->argument.type) {
	case OSC_INT32:
	case OSC_RGBA:
		color = message->argument.data.integer;
		break;
	case OSC_STRING:
		color = parse_hex_color(message->argument.data.string);
		if (color < 0) {
			printf(
			    "[osc] Wrong update for %s: %s.\n",
			    message->address,
			    message->argument.data.string
			);
			return;
		}
		break;
	default:
		printf("[osc] Unsupported type for %s update.\b", message->address);
		return;
	}
	update.color = color_from_int32_t(color);

	if (hid_push_update(&update) != PICO_OK) {
		printf("[osc] dropped update for %s.\n", message->address);
	}
}

void printf_event(encoder_event_t *event) {
	switch (event->ID.type) {
	case BUTTON:
		printf(
		    "Encoder %d%02d button: %s\n",
		    event->ID.row + 1,
		    event->ID.col + 1,
		    event->button ? "DOWN" : "UP"
		);
		break;
	case KNOB:
		printf(
		    "Encoder %d%02d knob: %d\n",
		    event->ID.row + 1,
		    event->ID.col + 1,
		    event->delta
		);
		break;
	case FADER:
		printf(
		    "Encoder %d%02d fader: %02x\n",
		    event->ID.row + 1,
		    event->ID.col + 1,
		    event->fader
		);
		break;
	}
}

void pull_encoder_events() {
	encoder_event_t event;

	while (true) {
		if (hid_pull_event(&event) != PICO_OK) {
			return;
		}
		send_event_to_osc(&event);
		// printf_event(&event);
	}
}

int main() {
	stdio_init_all();

	multicore_launch_core1(core1_main);
	while (multicore_lockout_victim_is_initialized(1) == false) {
		tight_loop_contents();
	}

	printf("Woodpecker\n");

	if (netusb_init() == false) {
		printf("[netusb]:could not initialize netusb!\n");
		return 1;
	}

	int err = osc_init();
	if (err != ERR_OK) {
		printf("[osc]: initialization failed: %d\n", err);
	}

	while (true) {
		netusb_task();
		pull_encoder_events();
	}

	return 0;
}
