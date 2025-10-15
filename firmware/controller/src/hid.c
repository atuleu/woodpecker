#include "hid.h"
#include "section_rx.h"
#include <hardware/irq.h>
#include <pico/error.h>
#include <pico/platform/sections.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>

#define BAUDRATE                  230400
#define BACKGROUND_TASK_PERIOD_us 250
#define SECOND_us                 (1000 * 1000)
#define NUM_RECEIVERS             2

_Static_assert(sizeof(section_frame_t) == 7, "Size of frame should be 7");

struct section_receiver {
	section_rx_t uart;

	size_t disp;
};
typedef struct section_receiver section_receiver_t;

int section_receiver_init(section_receiver_t *rx, int pin, int baudrate) {
	int err = section_rx_init(&rx->uart, pin, baudrate);
	if (err != PICO_OK) {
		return err;
	}
	rx->disp = 0;
	return PICO_OK;
}

void section_receiver_deinit(section_receiver_t *section) {
	section_rx_deinit(&section->uart);
}

inline static void section_frame_printf(section_frame_t *frame) {}

inline static void
hid_section_handle_frame(section_receiver_t *section, section_frame_t *frame) {}

static void section_receiver_work(section_receiver_t *section) {
	bool disp =
	    (section->disp++ % (SECOND_us / BACKGROUND_TASK_PERIOD_us)) == 0;

	section_rx_check_and_unblock(&section->uart);

	section_frame_t frame;
	while (true) {
		int res = section_rx_get(&section->uart, &frame);
		if (res == 0) {
			break;
		}

		hid_section_handle_frame(section, &frame);
		if (disp) {
			section_frame_printf(&frame);
		}
	}

	if (disp) {
		section_rx_stats_t stats;
		section_rx_get_stats(&section->uart, &stats);
		printf(
		    "Got receiver stats for pin %d: rx: %d rx_error: %d frame_error:%d "
		    "crc_error:%d\n",
		    section->uart.pin,
		    stats.received,
		    stats.locked_errors,
		    stats.framing_errors,
		    stats.crc_errors
		);
	}
}

struct hid {
	int               irq;
	repeating_timer_t timer;

	struct section_receiver receivers[NUM_RECEIVERS];
	encoder_t               encoders[HID_NUM_ENCODERS];
};

static struct hid sections = {
    .irq   = -1,
    .timer = {.user_data = NULL},
};

static void __isr __not_in_flash_func(hid_section_irq_handler)(void) {
	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		section_receiver_work(&sections.receivers[i]);
	}
	irq_clear(sections.irq);
}

static bool hid_section_repeated_timer(__unused repeating_timer_t *rt) {
	irq_set_pending(sections.irq);
	return true;
}

void _hid_sections_deinit(size_t sections);

int hid_init() {
	if (sections.irq >= 0) {
		return PICO_ERROR_INVALID_STATE;
	}

	int pins[6] = {2, 3, 4, 23, 24, 25};
	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		int err =
		    section_receiver_init(&sections.receivers[i], pins[i], BAUDRATE);
		if (err != PICO_OK) {
			_hid_sections_deinit(i);
			return err;
		}
	}

	sections.irq = user_irq_claim_unused(false);
	if (sections.irq < 0) {
		_hid_sections_deinit(NUM_RECEIVERS);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}
	irq_set_exclusive_handler(sections.irq, hid_section_irq_handler);

	irq_set_enabled(sections.irq, true);
	// sets the lowest priority for this user IRQ. we need it to be prempted.
	irq_set_priority(sections.irq, PICO_LOWEST_IRQ_PRIORITY);
	if (add_repeating_timer_us(
	        250,
	        hid_section_repeated_timer,
	        &sections,
	        &sections.timer
	    ) == false) {
		_hid_sections_deinit(NUM_RECEIVERS);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	return PICO_OK;
}

void hid_sections_deinit() {
	_hid_sections_deinit(NUM_RECEIVERS);
}

void _hid_sections_deinit(size_t num_sections) {
	if (sections.irq >= 0) {
		irq_set_enabled(sections.irq, false);
		irq_remove_handler(sections.irq, hid_section_irq_handler);
	}

	if (sections.timer.user_data != NULL) {
		cancel_repeating_timer(&sections.timer);
	}

	for (size_t i = 0; i < num_sections; ++i) {
		section_receiver_deinit(&sections.receivers[i]);
	}
}
