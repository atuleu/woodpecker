#pragma once

#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

#include "encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HID_NUM_SECTIONS
#define HID_NUM_SECTIONS 3
#endif

#define HID_NUM_ENCODERS (20 * HID_NUM_SECTIONS)

#define HID_ENCODER_IDX(row, col) ((row) * HID_NUM_SECTIONS * 5 + (col))

#ifndef HID_PRINT_STATS
#ifndef NDEBUG
#define HID_PRINT_STATS 1
#else
#define HID_PRINT_STATS 0
#endif // NDEBUG
#endif // HID_PRINT_STATS

/**
 * Inits the HID submodule. The HID submodule handles incoming packet from
 * daughter board, emits encoder_event_t, and manage incoming encoder color
 * updates.
 */
int hid_init();

/** Deinit the HID submodule */
void hid_deinit();

/** Task for HID to be run periodically. This is much a cleanup update
 * steps. All encoder events are pulled in a background task to minimize
 * jitter. The background task is run on a low priority interrupt line.
 */
void hid_task();

/** get the current state of the encoder. Safe to call from the same core that
 * hid_init() was called. */
int hid_get_state(encoder_t *encoders, size_t len);

/** Gets all pending events. Safe to call from any core. Avoid to call from
 * interrupt context (but safe to do so). */
int hid_pull_event(encoder_event_t *event);

/**
 * Pushes an encoder update. Safe to call from any core. Avoid to call from
 * interrupt context (but safe to do so).
 */
int hid_push_update(encoder_update_t *update);

#ifdef __cplusplus
}
#endif
