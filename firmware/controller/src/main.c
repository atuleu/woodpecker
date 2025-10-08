
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include <pico/error.h>
#include <pico/platform/common.h>
#include <pico/stdlib.h>

#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>

#include "i2c_dma.h"
#include "lp5864.h"
#include "netusb.h"

#define PERIOD_ticks (6 * 256)
#define PERIOD_us    (50 * 1000)
#define BAUDRATE     400 * 1000

static bool schedule_render = true;

void render(
    uint32_t now_ms, i2c_dma_inst_t *top, i2c_dma_inst_t *bot, bool schedule
) {
	uint8_t dots[144 * 3];
	// makes a rainbow effect
	for (uint i = 0; i < 144 * 3; ++i) {
		uint32_t phase =
		    ((i / 3) * 50 + i * 2 * 256 + now_ms / 2) % PERIOD_ticks;
		if (phase < 256) {
			dots[i] = phase;
		} else if (phase < 3 * 256) {
			dots[i] = 255;
		} else if (phase < 4 * 256) {
			dots[i] = 4 * 256 - 1 - phase;
		} else {
			dots[i] = 0;
		}
	}
	static i2c_dma_xmit_id top_xmit, bot_xmit;
	for (uint8_t addr = 0; addr < 1; ++addr) {
		if (schedule) {
			int err = lp5864_schedule_write(
			    top,
			    addr,
			    0x200,
			    dots + 0 + 144 * addr,
			    72,
			    &top_xmit
			);
			if (err != PICO_OK) {
				printf(
				    "could not schedule top[%d] write last: %d err: %d!\n",
				    addr,
				    top_xmit,
				    err
				);
			}
			err = lp5864_schedule_write(
			    bot,
			    addr,
			    0x200,
			    dots + 72 + 144 * addr,
			    72,
			    &bot_xmit
			);
			if (err != PICO_OK) {
				printf(
				    "could not schedule bot[%d] write last: %d err:%d!\n",
				    addr,
				    bot_xmit,
				    err
				);
			}
		} else {
			lp5864_write_blocking(i2c0, addr, 0x200, dots + addr * 144, 72);
			lp5864_write_blocking(
			    i2c1,
			    addr,
			    0x200,
			    dots + addr * 144 + 72,
			    72
			);
		}
	}
	/* while (i2c_dma_xmit_done(top, top_xmit) && i2c_dma_xmit_done(bot,
	 * bot_xmit) */
	/* ) { */
	/* 	tight_loop_contents(); */
	/* } */
}

int main() {
	stdio_init_all();

	printf("Woodpecker hello-world\n");

	/* if (netusb_init() == false) { */
	/* 	printf("could not initialize netusb. Bye!\n"); */
	/* } */

	/* while (true) { */
	/* 	netusb_task(); */
	/* } */
	/* netusb_deinit(); */

	printf("Attempt to read the I2C chip\n");

	i2c_dma_inst_t *top_bus = i2c_dma_init(i2c0, BAUDRATE, 12, 9);
	if (top_bus == NULL) {
		printf("Unable to initialize top I2C\n");
		return 1;
	}
	i2c_dma_inst_t *bot_bus = i2c_dma_init(i2c1, BAUDRATE, 22, 19);
	if (bot_bus == NULL) {
		printf("Unable to initialize bot I2C\n");
		return 1;
	}

	uint8_t buffer[2];
	buffer[0] = 0x01;
	buffer[1] = (4 << 3);
	printf("Writing Top Config: ");
	int res = lp5864_write_blocking(i2c0, 0, 0, buffer, 2);
	printf("%s\n", res == 2 ? " OK" : "ERR");

	printf("Writing Bot Config: ");
	res = lp5864_write_blocking(i2c1, 0, 0, buffer, 2);
	printf("%s\n", res == 2 ? " OK" : "ERR");

	struct LP5864_Current_Compensation config = {
	    .Group1 = 90,
	    .Group2 = 40,
	    .Group3 = 127,
	};

	printf("Writing Top CC: ");
	res = lp5864_write_blocking(i2c0, 0, LP5864_CC_ADDRESS, &config, 3);
	printf("%s\n", res == 3 ? " OK" : "ERR");

	printf("Writing Bot CC: ");
	res = lp5864_write_blocking(i2c1, 0, LP5864_CC_ADDRESS, &config, 3);
	printf("%s\n", res == 3 ? " OK" : "ERR");

	absolute_time_t last_update = -PERIOD_us;
	while (true) {
		i2c_dma_xmit_id failed = i2c_dma_check_and_failed_stalled(top_bus);
		if (failed != PICO_OK) {
			printf("Top xmit %d failed.\n", failed);
		}
		failed = i2c_dma_check_and_failed_stalled(bot_bus);
		if (failed != PICO_OK) {
			printf("Bot xmit %d failed.\n", failed);
		}

		absolute_time_t now = get_absolute_time();
		if (absolute_time_diff_us(last_update, now) < PERIOD_us) {
			continue;
		}
		render(now / 1000, top_bus, bot_bus, schedule_render);
		last_update += PERIOD_us;
	}

	/* res = lp5864_write_blocking( */
	/*     i2c0, */
	/*     0, */
	/*     0x200, */
	/*     dots, */
	/*     sizeof(dots) / sizeof(uint8_t) */
	/* ); */
	/* printf( */
	/*     "%s %d\n", */
	/*     res == sizeof(dots) / sizeof(uint8_t) ? " OK" : "ERR", */
	/*     sizeof(dots) / sizeof(uint8_t) */
	/* ); */
	return 0;
}
