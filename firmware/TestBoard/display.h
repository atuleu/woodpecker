#pragma once

#include <pico/stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	void display_init();

	void display_set_pixel(uint8_t x, uint8_t y,
						   uint8_t r, uint8_t g, uint8_t b);

	void display_process();

#ifdef __cplusplus
}
#endif
