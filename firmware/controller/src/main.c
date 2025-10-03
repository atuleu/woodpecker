
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include <pico/error.h>
#include <pico/stdlib.h>

#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>

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

	uint8_t buffer[3];
	buffer[0] = 1;
#define ADDRESS 0b01000000
	int res = i2c_write_blocking(i2c0, ADDRESS, buffer, 1, true);
	if (res == PICO_ERROR_GENERIC || res != 1) {
		printf("Got an error on write\n");
		return 1;
	}
	res = i2c_read_blocking(i2c0, ADDRESS, buffer, 3, false);
	if (res == PICO_ERROR_GENERIC) {
		printf("Got an error on write\n");
		return 1;
	}
	if (res < 3) {
		printf("Could only read %d bytes.\n", res);
	}
	printf("Result: 0x%03X 0x%03X 0x%03X\n", buffer[0], buffer[1], buffer[2]);
}
