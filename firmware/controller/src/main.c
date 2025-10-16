#include <hardware/sync.h>
#include <lwip/err.h>
#include <pico/util/queue.h>
#include <stdint.h>
#include <stdio.h>

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/error.h>
#include <pico/multicore.h>
#include <pico/platform/common.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/types.h>

#include "atomic.h"
#include "encoder.h"
#include "hid.h"
#include "hid_visual.h"
#include "i2c_dma.h"
#include "lp5864.h"
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
		if (hid_pull_event(&event) == 0) {
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
