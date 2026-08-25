#include "hid.h"

#include <pico/types.h>
#include <stdio.h>

#include <hardware/irq.h>
#include <pico/error.h>
#include <pico/platform/sections.h>
#include <pico/time.h>
#include <pico/util/queue.h>
#include <string.h>

#include "atomic.h"
#include "encoder.h"
#include "osc.h"
#include "section_rx.h"

#define BAUDRATE                  230400
#define BACKGROUND_TASK_PERIOD_us 300
#define SECOND_us                 (1000 * 1000)
#define NUM_RECEIVERS             6
#define STATS_UPDATE_PERIOD_us    (1000 * 1000)

#ifndef NDEBUG
#include "ctime_prob.h"
#endif

struct hid {
	int               irq;
	alarm_pool_t     *alarm_pool;
	repeating_timer_t timer;

	section_rx_t    receivers[NUM_RECEIVERS];
	encoder_t       encoders[HID_NUM_ENCODERS];
	queue_t         events, updates;
	absolute_time_t last_update;
};

static struct hid sections = {
    .irq   = -1,
    .timer = {.user_data = NULL},
};

inline static void section_frame_printf(section_frame_t *frame) {}

inline static void hid_section_handle_frame(
    section_rx_t *rx, section_frame_t *frame, absolute_time_t now
) {
	if (frame->ID >= HID_NUM_SECTIONS) {
		return;
	}
	size_t col_offset = 5 * frame->ID;

	encoder_event_t event;

#define may_push(expr)                                                         \
	do {                                                                       \
		int res = expr;                                                        \
		if (res != 0) {                                                        \
			queue_try_add(&sections.events, &event);                           \
		}                                                                      \
	} while (0)

	switch (frame->Type) {
	case FRAME_ENCODER:
		for (size_t i = 0; i < 5; ++i) {
			may_push(encoder_push_button(
			    &sections.encoders[HID_ENCODER_IDX(3, col_offset + i)],
			    frame->Buttons & (1 << i),
			    now,
			    &event
			));
			may_push(encoder_push_knob(
			    &sections.encoders[HID_ENCODER_IDX(3, col_offset + i)],
			    (i & 0x01) ? frame->Encoder[i / 2].B : frame->Encoder[i / 2].A,
			    now,
			    &event
			));

			size_t j = i + 5;
			may_push(encoder_push_button(
			    &sections.encoders[HID_ENCODER_IDX(2, col_offset + i)],
			    frame->Buttons & (1 << j),
			    now,
			    &event
			));
			may_push(encoder_push_knob(
			    &sections.encoders[HID_ENCODER_IDX(2, col_offset + i)],
			    (j & 0x01) ? frame->Encoder[j / 2].B : frame->Encoder[j / 2].A,
			    now,
			    &event
			));
		}
		break;
	case FRAME_FADER:
		for (size_t i = 0; i < 5; ++i) {
			may_push(encoder_push_button(
			    &sections.encoders[HID_ENCODER_IDX(0, col_offset + i)],
			    frame->Buttons & (1 << i),
			    now,
			    &event
			));

			may_push(encoder_push_fader(
			    &sections.encoders[HID_ENCODER_IDX(1, col_offset + i)],
			    frame->Fader[i],
			    now,
			    &event
			));

			size_t j = i + 5;
			may_push(encoder_push_button(
			    &sections.encoders[HID_ENCODER_IDX(1, col_offset + i)],
			    frame->Buttons & (1 << j),
			    now,
			    &event
			));
		}
		break;
	}
}

static void section_receiver_work(section_rx_t *rx, absolute_time_t now) {

	section_frame_t frame;
	while (true) {
		int res = section_rx_get(rx, &frame);
		if (res == 0) {
			break;
		}

		hid_section_handle_frame(rx, &frame, now);
	}
}

#ifndef NDEBUG
static ctime_prob_t *_ctime = NULL;
#endif

static void __isr __not_in_flash_func(hid_section_irq_handler)(void) {
#ifndef NDEBUG
	if (_ctime == NULL) {
		_ctime = ctime_prob_init(4096, "hid_background");
	}
#endif
	absolute_time_t now = get_absolute_time();
	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		section_receiver_work(&sections.receivers[i], now);
	}
	irq_clear(sections.irq);

#ifndef NDEBUG
	ctime_prob_push(_ctime, absolute_time_diff_us(now, get_absolute_time()));
#endif
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

	for (size_t i = 0; i < HID_NUM_SECTIONS * 5; ++i) {
		encoder_init(
		    &sections.encoders[HID_ENCODER_IDX(0, i)],
		    (struct encoder_ID){.type = BUTTON, .row = 0, .col = i}
		);
		encoder_init(
		    &sections.encoders[HID_ENCODER_IDX(1, i)],
		    (struct encoder_ID){.type = FADER, .row = 1, .col = i}
		);
		encoder_init(
		    &sections.encoders[HID_ENCODER_IDX(2, i)],
		    (struct encoder_ID){.type = KNOB, .row = 2, .col = i}
		);
		encoder_init(
		    &sections.encoders[HID_ENCODER_IDX(3, i)],
		    (struct encoder_ID){.type = KNOB, .row = 3, .col = i}
		);
	}

	queue_init(&sections.events, sizeof(encoder_event_t), 64);
	queue_init(&sections.updates, sizeof(encoder_update_t), 64);

	int pins[6] = {2, 3, 4, 25, 24, 23};
	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		int err = section_rx_init(&sections.receivers[i], pins[i], BAUDRATE);
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

	if (get_core_num() == 0) {
		alarm_pool_init_default();
		sections.alarm_pool = alarm_pool_get_default();
	} else {
		sections.alarm_pool = alarm_pool_create_with_unused_hardware_alarm(1);
	}

	if (alarm_pool_add_repeating_timer_us(
	        sections.alarm_pool,
	        250,
	        hid_section_repeated_timer,
	        &sections,
	        &sections.timer
	    ) == false) {
		_hid_sections_deinit(NUM_RECEIVERS);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	sections.last_update = -STATS_UPDATE_PERIOD_us;
	return PICO_OK;
}

void hid_sections_deinit() {
	_hid_sections_deinit(NUM_RECEIVERS);
}

void _hid_sections_deinit(size_t num_sections) {
	if (sections.irq >= 0) {
		irq_set_enabled(sections.irq, false);
		irq_remove_handler(sections.irq, hid_section_irq_handler);
		queue_free(&sections.events);
	}

	if (sections.timer.user_data != NULL) {
		cancel_repeating_timer(&sections.timer);
	}

	for (size_t i = 0; i < num_sections; ++i) {
		section_rx_deinit(&sections.receivers[i]);
	}
}

int hid_get_state(encoder_t *encoders, size_t len) {
	int res = (len < HID_NUM_ENCODERS) ? len : HID_NUM_ENCODERS;
	ATOMIC_CORE_BLOCK() {
		memcpy(encoders, sections.encoders, res * sizeof(encoder_t));
	}
	return res;
}

int hid_pull_event(encoder_event_t *event) {
	return queue_try_remove(&sections.events, event) ? PICO_OK
	                                                 : PICO_ERROR_LOCK_REQUIRED;
}

static size_t toDisplay = 0;

int hid_push_update(encoder_update_t *update) {
	return queue_try_add(&sections.updates, update) ? PICO_OK
	                                                : PICO_ERROR_LOCK_REQUIRED;
}

void _hid_pull_pending_updates() {
	encoder_update_t update;
	while (true) {
		if (queue_try_remove(&sections.updates, &update) == false) {
			break;
		}
		size_t idx = HID_ENCODER_IDX(update.ID.row, update.ID.col);
		if (idx >= HID_NUM_ENCODERS) {
			printf(
			    "[hid] Invalid encoder address %d%02d: ignoring update "
			    "%02x%02x%02x.\n",
			    update.ID.row + 1,
			    update.ID.col + 1,
			    update.color.R,
			    update.color.G,
			    update.color.B
			);
			continue;
		}
		ATOMIC_CORE_BLOCK() {
			sections.encoders[idx].color = update.color;
		}
	}
}

static inline const char *_hid_section_name(size_t i) {
	static const char *names[] =
	    {"top/0", "top/1", "top/2", "bot/0", "bot/1", "bot/2", "unknown"};
	if (i > 6) {
		i = 6;
	}
	return names[i];
}

void hid_task() {
	_hid_pull_pending_updates();

	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		section_rx_check_and_unblock(&sections.receivers[i]);
	}

#if HID_PRINT_STATS == 1
	absolute_time_t now = get_absolute_time();
	if (absolute_time_diff_us(sections.last_update, now) <
	    STATS_UPDATE_PERIOD_us) {
		return;
	}

	size_t periods = 0;
	while (absolute_time_diff_us(sections.last_update, now) >=
	       STATS_UPDATE_PERIOD_us) {
		periods += 1;
		sections.last_update += STATS_UPDATE_PERIOD_us;
	}
	if (periods > 1) {
		printf("[hid] stats update miss %d\n", periods - 1);
	}

	section_rx_stats_t stats;
	section_rx_get_stats(&sections.receivers[toDisplay], &stats);
	printf(
	    "[hid/receiver/%s] stats: received:%d locked_errors:%d "
	    "frame_errors:%d crc_errors:%d\n",
	    _hid_section_name(toDisplay),
	    stats.received,
	    stats.locked_errors,
	    stats.framing_errors,
	    stats.crc_errors
	);
	toDisplay += 1;
	if (toDisplay >= NUM_RECEIVERS) {
		toDisplay = 0;
	}
#endif
}

int hid_push_stats_over_osc() {
	for (size_t i = 0; i < NUM_RECEIVERS; ++i) {
		section_rx_stats_t stats;
		section_rx_get_stats(&sections.receivers[i], &stats);
		err_t err = osc_send_stats(_hid_section_name(i), &stats);
		if (err != ERR_OK) {
			printf(
			    "[hid] could not send stats for %s to OSC: %d\n",
			    _hid_section_name(i),
			    err
			);
			return PICO_ERROR_GENERIC;
		}
	}
	return PICO_OK;
}

void hid_mark_all_encoders(bool blinky) {
	for (size_t i = 0; i < HID_NUM_ENCODERS; ++i) {
		sections.encoders[i].blink = blinky;
	}
}
