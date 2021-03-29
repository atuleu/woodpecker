#include <stdio.h>
#include "pico/stdlib.h"

#include "tlc5973.pio.h"

static int i;

static inline void put_pixel(uint8_t r,uint8_t g, uint8_t b) {
	pio_sm_put_blocking(pio0,
						0,
						(47 << 16) | (0x3aa << 4 ) | r >> 4);
	pio_sm_put_blocking(pio0,
						0,
						(r << 28) | (g << 16) |(b << 4));
}

void clock_init() {
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);

	stdio_init_all();
	i = 0;
}

void clock_out() {
	gpio_put(PICO_DEFAULT_LED_PIN,i%2);
	i++;
	if ( (i % 2) == 0 ) {
		put_pixel(0xff,0x00,0x00);
		put_pixel(0x00,0xff,0x00);
		put_pixel(0x00,0x00,0xff);
		put_pixel(0xff,0xff,0x00);

		printf("It is %d seconds\n",i/2);
	} else {
		put_pixel(0x00,0x00,0x00);
		put_pixel(0x00,0x00,0x00);
		put_pixel(0x00,0x00,0x00);
		put_pixel(0x00,0x00,0x00);
	}
}

int main() {
	clock_init();
	uint64_t period = 500e3;
	uint64_t last = time_us_64();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &tlc5973_program);

    tlc5973_program_init(pio0, 0, offset, 10, 2e6);

	while(true) {
		uint64_t now = time_us_64();
		if ( (now - last) >= period ) {
			last += period;
			clock_out();
		}
	}
	return 0;
}
