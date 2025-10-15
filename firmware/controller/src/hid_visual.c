#include <pico/error.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "encoder.h"
#include "hid.h"
#include "hid_visual.h"
#include "i2c_dma.h"
#include "lp5864.h"

#define I2C_BAUDRATE     (400 * 1000)
#define TOP_SDA_PIN      12
#define TOP_SCL_PIN      9
#define BOT_SDA_PIN      22
#define BOT_SCL_PIN      19
#define UPDATE_PERIOD_us (30 * 1000)

#define STARTUP_ANIMATE_DURATION_us (2 * 1000 * 1000)
#define STARTUP_ANIMATE_DURATION_ms (2 * 1000)
#define NUM_PIXELS                  (6 * 8 * HID_NUM_SECTIONS)
#define NUM_DOTS                    (3 * NUM_PIXELS)
#define norm2(x, y)                 ((x) * (x) + (y) * (y))
#define MAX_DIST                    norm2(6 * HID_NUM_SECTIONS, 8)
#define UPDATE_ANIMATION_ms         (uint16_t)(2 * 1000)
#define UPDATE_ANIMATION_us         (uint64_t)(UPDATE_ANIMATION_ms * 1000)

struct hid_visual {
	i2c_dma_t      *top_bus, *bot_bus;
	absolute_time_t last_update;
};

static struct hid_visual visuals;

int _hid_visual_schedule_lp5864_config(
    i2c_dma_t *bus, uint8_t address, i2c_dma_xmit_id *xmit
) {
	union {
		struct __attribute__((packed)) {
			struct LP5864_Chip_en     Chip_en;
			struct LP5864_Dev_initial Dev_initial;
		} as_struct;

		uint8_t as_buffer[2];
	} config_gen;

	union {
		struct LP5864_Current_Compensation as_CC;
		uint8_t                            as_buffer[3];
	} config_CC;

	config_gen.as_struct.Chip_en =
	    (struct LP5864_Chip_en){.EN = 1, ._reserved = 0};
	config_gen.as_struct.Dev_initial = (struct LP5864_Dev_initial
	){.PWM_Frequency = 0, .Data_Ref_Mode = 0, .Max_Line_Num = 4};

	config_CC.as_CC.Group1 = 90;  // Blue
	config_CC.as_CC.Group2 = 40;  // Green
	config_CC.as_CC.Group3 = 127; // Red

	int err = lp5864_schedule_write(
	    bus,
	    address,
	    LP5864_Chip_en_ADDRESS,
	    config_gen.as_buffer,
	    2,
	    xmit
	);
	if (err != PICO_OK) {
		return err;
	}

	err = lp5864_schedule_write(
	    bus,
	    address,
	    LP5864_CC_ADDRESS,
	    config_CC.as_buffer,
	    3,
	    xmit + 1
	);
	return err;
}

int hid_visual_init() {
	visuals.top_bus =
	    i2c_dma_init(i2c0, I2C_BAUDRATE, TOP_SDA_PIN, TOP_SCL_PIN);
	if (visuals.top_bus == NULL) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	visuals.bot_bus =
	    i2c_dma_init(i2c1, I2C_BAUDRATE, BOT_SDA_PIN, BOT_SCL_PIN);
	if (visuals.bot_bus == NULL) {
		i2c_dma_deinit(visuals.top_bus);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	i2c_dma_xmit_id xmit_top[2 * HID_NUM_SECTIONS],
	    xmit_bot[2 * HID_NUM_SECTIONS];

	printf("[hid_visuals] Initializing LP5864 chips\n");

	for (size_t i = 0; i < HID_NUM_SECTIONS; ++i) {
		_hid_visual_schedule_lp5864_config(
		    visuals.top_bus,
		    i,
		    xmit_top + 2 * i
		);
		_hid_visual_schedule_lp5864_config(
		    visuals.bot_bus,
		    i,
		    xmit_bot + 2 * i
		);
	}

	i2c_dma_xmit_wait(visuals.top_bus, xmit_top[2 * HID_NUM_SECTIONS - 1]);
	i2c_dma_xmit_wait(visuals.bot_bus, xmit_bot[2 * HID_NUM_SECTIONS - 1]);

	for (size_t i = 0; i < 2 * HID_NUM_SECTIONS; ++i) {
		i2c_dma_xmit_status s =
		    i2c_dma_xmit_get_status(visuals.top_bus, xmit_top[i]);
		if (s & I2C_DMA_XMIT_ABORTED) {
			printf(
			    "[hid_visual] Could not set config for Top LP5864 ID=%d\n",
			    i / 2
			);
		}
		s = i2c_dma_xmit_get_status(visuals.bot_bus, xmit_bot[i]);
		if (s & I2C_DMA_XMIT_ABORTED) {
			printf(
			    "[hid_visual] Could not set config for Bot LP5864 ID=%d\n",
			    i / 2
			);
		}
	}

	visuals.last_update = -UPDATE_PERIOD_us;
	return PICO_OK;
}

void hid_visual_deinit() {
	i2c_dma_deinit(visuals.top_bus);
	i2c_dma_deinit(visuals.bot_bus);
}

union hid_update {
	struct {
		uint8_t B;
		uint8_t G;
		uint8_t R;
	} pixels[NUM_PIXELS];

	uint8_t dots[NUM_DOTS];
};

static inline void
_hid_set_pixel(union hid_update *update, size_t i, color_t color) {
	update->pixels[i].B = color.B;
	update->pixels[i].G = color.G;
	update->pixels[i].R = color.R;
}

int _hid_schedule_update(union hid_update *update) {
	for (size_t i = 0; i < HID_NUM_SECTIONS; ++i) {
		i2c_dma_xmit_id xmit;
		int             err = lp5864_schedule_write(
            visuals.top_bus,
            i,
            LP5864_DOT_PWM(0, 0),
            update->dots + 144 * i,
            72,
            &xmit
        );
		if (err != PICO_OK) {
			return err;
		}
		err = lp5864_schedule_write(
		    visuals.bot_bus,
		    i,
		    LP5864_DOT_PWM(0, 0),
		    update->dots + 72 + 144 * i,
		    72,
		    &xmit
		);
		if (err != PICO_OK) {
			return err;
		}
	}
	return PICO_OK;
}

void _hid_animate_startup(uint32_t now_ms) {
	union hid_update d;

	size_t x = -1;
	size_t y = 0;
	for (size_t i = 0; i < NUM_PIXELS; ++i) {
		++x;
		if (x == HID_NUM_SECTIONS * 6) {
			x = 0;
			++y;
		}

		uint32_t phase =
		    STARTUP_ANIMATE_DURATION_ms / 2 -
		    norm2(x, y) * STARTUP_ANIMATE_DURATION_ms / (2 * MAX_DIST);
		if (phase < 255) {
			_hid_set_pixel(&d, i, (color_t){.R = 0, .G = phase, .B = phase});
		} else if (phase < (1000 - 255)) {
			_hid_set_pixel(&d, i, (color_t){.R = 0, .G = 255, .B = 255});
		} else if (phase < 1000) {
			_hid_set_pixel(
			    &d,
			    i,
			    (color_t){.R = 0, .G = 1000 - phase, .B = 1000 - phase}
			);
		} else {
			_hid_set_pixel(&d, i, (color_t){.R = 0, .G = 0, .B = 0});
		}
	}

	int err = _hid_schedule_update(&d);
	if (err != PICO_OK) {
		printf("[hid_visual] Could not schedule update %d\n", err);
	}
}

color_t color_mult(uint16_t frac_up, uint16_t frac_down, color_t c) {
#define scale(v) (uint32_t)(MIN(255, ((uint32_t)(v)*frac_up) / frac_down))
	return (color_t){.R = scale(c.R), .G = scale(c.G), .B = scale(c.B)};
}

void hid_render(encoder_t *encoders, size_t num_encoders, absolute_time_t now) {
	union hid_update d;
	memset(d.dots, 0, NUM_DOTS);
	for (size_t i = 0; i < num_encoders; ++i) {
		encoder_t *enc = &encoders[i];
		if (enc->ID.col >= 5 * HID_NUM_SECTIONS) {
			continue;
		}
		color_t c;
		size_t  pixel_col = (enc->ID.col / 5) * 6 + enc->ID.col % 5;
		int64_t ellapsed  = absolute_time_diff_us(enc->last_change, now);
		if (ellapsed > 0 && ellapsed < UPDATE_ANIMATION_us) {
			c = color_mult(
			    2 * UPDATE_ANIMATION_ms - ellapsed / 1000,
			    2 * UPDATE_ANIMATION_ms,
			    enc->color
			);
		} else {
			c = color_mult(1, 2, enc->color);
		}

		switch (encoders->ID.row) {
		case 0:
			_hid_set_pixel(&d, 7 * 6 * HID_NUM_SECTIONS + pixel_col, c);
			break;
		case 1:
			_hid_set_pixel(&d, 6 * 6 * HID_NUM_SECTIONS + pixel_col, c);
			break;
		case 2:
			_hid_set_pixel(&d, 3 * 6 * HID_NUM_SECTIONS + pixel_col, c);
			_hid_set_pixel(&d, 2 * 6 * HID_NUM_SECTIONS + pixel_col, c);
			break;
		case 3:
			_hid_set_pixel(&d, 1 * 6 * HID_NUM_SECTIONS + pixel_col, c);
			_hid_set_pixel(&d, pixel_col, c);
			break;
		}
	}
	int err = _hid_schedule_update(&d);
	if (err != PICO_OK) {
		printf("[hid_visual] Could not schedule update %d\n", err);
	}
}

void _hid_render_rainbow(
    uint32_t now_ms, i2c_dma_t *top, i2c_dma_t *bot, bool schedule
) {
#define CHANNEL_DEPHASE 51
#define PERIOD_ticks    (6 * 256)

	union hid_update d;
	// makes a rainbow effect
	for (uint i = 0; i < 144 * 3; ++i) {
		uint32_t phase =
		    ((i / 3) * 50 + i * CHANNEL_DEPHASE + now_ms / 2) % PERIOD_ticks;
		if (phase < 256) {
			d.dots[i] = phase;
		} else if (phase < 3 * 256) {
			d.dots[i] = 255;
		} else if (phase < 4 * 256) {
			d.dots[i] = 4 * 256 - 1 - phase;
		} else {
			d.dots[i] = 0;
		}
	}
	int err = _hid_schedule_update(&d);
	if (err != PICO_OK) {
		printf("[hid_visual] Could not schedule update %d\n", err);
	}
}

void _hid_render_full(
    uint32_t now_ms, i2c_dma_t *top, i2c_dma_t *bot, bool schedule
) {
	union hid_update d;
	memset(d.dots, 0xff, NUM_DOTS);
	int err = _hid_schedule_update(&d);
	if (err != PICO_OK) {
		printf("[hid_visual] Could not schedule update %d\n", err);
	}
}

#define WINDOW_SIZE 64
static int64_t durations[WINDOW_SIZE];
size_t renders = 0;

void _hid_perform_render(absolute_time_t now) {
	if (now < from_us_since_boot(STARTUP_ANIMATE_DURATION_us)) {
		_hid_animate_startup(now / 1000);
		return;
	}

	encoder_t encoders[HID_NUM_ENCODERS];
	hid_get_state(encoders, HID_NUM_ENCODERS);

	hid_render(encoders, HID_NUM_ENCODERS, now);
}

void _hid_visual_push_ctime(absolute_time_t start, absolute_time_t end) {
	durations[renders++] = absolute_time_diff_us(start, end);
	renders &= WINDOW_SIZE - 1;
	if (renders != 0) {
		return;
	}
	int64_t max  = 0;
	int64_t min  = 1LL << 62;
	int64_t mean = 0;
	for (size_t j = 0; j < WINDOW_SIZE; ++j) {
		min = MIN(durations[j], min);
		max = MAX(durations[j], max);
		mean += durations[j];
	}
	mean /= WINDOW_SIZE;
	printf(
	    "[hid_visual] render duration mean=%lld.%03lldms min=%lld.%03lld "
	    "max=%lld.%03lld\n",
	    mean / 1000,
	    mean % 1000,
	    min / 1000,
	    min % 1000,
	    max / 1000,
	    max % 1000
	);
}

void hid_visual_task() {
	absolute_time_t now = get_absolute_time();
	if (absolute_time_diff_us(visuals.last_update, now) < UPDATE_PERIOD_us) {
		return;
	}

	size_t periods;
	for (periods = 0; visuals.last_update < now; ++periods) {
		visuals.last_update += UPDATE_PERIOD_us;
	}
	if (periods > 1) {
		printf("[hid_visual]: missed %d update.\n", periods - 1);
	}
	_hid_visual_push_ctime(now, get_absolute_time());
}
