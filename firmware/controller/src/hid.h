#pragma once

#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

#include "encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HID_NUM_SECTIONS
#define HID_NUM_SECTIONS 1
#endif

#define HID_NUM_ENCODERS (20 * HID_NUM_SECTIONS)

int hid_init();

void hid_deinit();

void hid_task();

int hid_get_state(encoder_t *encoders, size_t len);

int hid_pull_event(encoder_event_t *event);

#ifdef __cplusplus
}
#endif
