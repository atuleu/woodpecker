
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include <pico/error.h>
#include <pico/platform/common.h>
#include <pico/stdlib.h>

#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>

#include "i2c_dma.h"
#include "lp5864.h"
#include "netusb.h"

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
	gpio_set_function(9, GPIO_FUNC_I2C);
	gpio_set_function(12, GPIO_FUNC_I2C);

	i2c_init(i2c0, 400 * 1000);
	i2c_dma_inst_t *bus = i2c_dma_init(i2c0);
	if (bus == NULL) {
		printf("Unable to initialize DMA\n");
		return 1;
	}

	uint8_t buffer[3];
	buffer[0] = 1;
	i2c_dma_xmit_id xmit;

	printf("Reading chip base address: ");
	int res = lp5864_schedule_read(bus, 0, 0, buffer, 3, &xmit);
	if (res != PICO_OK) {
		printf("ERR\nCould not schedule read\n");
		return 1;
	}
	while (i2c_dma_xmit_done(bus, xmit) == false) {
		tight_loop_contents();
	}
	printf("OK\n");
	printf(
	    "Result: %d 0x%03X 0x%03X 0x%03X\n",
	    res,
	    buffer[0],
	    buffer[1],
	    buffer[2]
	);
	buffer[0] = 0x01;
	buffer[1] = (4 << 3);
	printf("Writing Config: ");
	res = lp5864_write_blocking(i2c0, 0, 0, buffer, 2);
	printf("%s\n", res == 2 ? " OK" : "ERR");

	printf("Writing CC: ");
	res = lp5864_write_blocking(
	    i2c0,
	    0,
	    LP5864_CC_ADDRESS,
	    &(struct LP5864_Current_Compensation){
	        .Group1 = 90,
	        .Group2 = 48,
	        .Group3 = 127,
	    },
	    3
	);
	printf("%s\n", res == 3 ? " OK" : "ERR");

	// clang-format off
	uint8_t dots[] = {
		0xff, 0x00, 0x00, // L0 - C1
		0x00, 0xff, 0x00, // L0 - C2
		0x00, 0x00, 0xff, // L0 - C3
		0xff, 0xff, 0xff, // L0 - C4
		0xff, 0xff, 0xff, // L0 - C5
		0x00, 0x00, 0x00, // unused
		0xff, 0x00, 0x00, // L1 - C1
		0x00, 0xff, 0x00, // L1 - C2
		0x00, 0x00, 0xff, // L1 - C3
		0xff, 0xff, 0xff, // L1 - C4
		0xff, 0xff, 0xff, // L1 - C5
		0x00, 0x00, 0x00, // unused
		0xff, 0xff, 0x00, // L2 - C1
		0xff, 0x00, 0xff, // L2 - C2
		0x00, 0xff, 0xff, // L2 - C3
		0xff, 0xff, 0xff, // L2 - C4
		0xff, 0xff, 0xff, // L2 - C5
		0x00, 0x00, 0x00, // unused
		0xff, 0xff, 0x00, // L3 - C1
		0xff, 0x00, 0xff, // L3 - C2
		0x00, 0xff, 0xff, // L3 - C3
		0xff, 0xff, 0xff, // L3 - C4
		0xff, 0xff, 0xff, // L3 - C5
	    0x00, 0x00, 0x00, // unused
	};
	// clang-format on
	printf("Printing dots: ");
	res = lp5864_schedule_write(bus, 0, 0x200, dots, sizeof(dots), &xmit);
	if (res != PICO_OK) {
		printf("ERR\nCould not schedule write.\n");
		return 1;
	}
	while (i2c_dma_xmit_done(bus, xmit) == false) {
		tight_loop_contents();
	}
	printf("OK\n");

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
