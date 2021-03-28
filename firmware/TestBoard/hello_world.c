#include <stdio.h>
#include "pico/stdlib.h"

static int i;

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
		printf("It is %d seconds\n",i/2);
	}
}

int main() {
	clock_init();
	uint64_t period = 500e3;
	uint64_t last = time_us_64();

	while(true) {
		uint64_t now = time_us_64();
		if ( (now - last) >= period ) {
			last += period;
			clock_out();
		}
	}
	return 0;
}
