#include <hardware/sync.h>
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

#include "encoder.h"
#include "hid.h"
#include "hid_visual.h"
#include "i2c_dma.h"
#include "lp5864.h"
#include "netusb.h"

#define PERIOD_us (20 * 1000)
#define BAUDRATE  400 * 1000
#define WAIT      false

static bool schedule_render = true;

void core1_main() {
	multicore_lockout_victim_init();

	int err = hid_init();
	if (err != PICO_OK) {
		printf("[hid] Initialization failed: %d\n", err);
		return;
	}
	printf("[hid] Initialized.\n");

	err = hid_visual_init();
	if (err != PICO_OK) {
		printf("[hid_visual] Initialization failed:%d\n", err);
		return;
	}
	printf("[hid_visual] Initialized.\n");
	while (true) {
		hid_visual_task();
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
		printf_event(&event);
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

	while (true) {
		netusb_task();
		pull_encoder_events();
	}

	return 0;
}
