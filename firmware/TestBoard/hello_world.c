#include <stdio.h>
#include "pico/stdlib.h"

#include "display.h"

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
	} else {

	}
}
static uint8_t pixels[5*3*6];
typedef void(*animatorFunc)();
static animatorFunc animator;
void go_red();
void go_yellow();
void go_green();
void go_cyan();
void go_blue();
void go_magenta();

void go_yellow() {
	if ( ++pixels[1] == 255 ) {
		animator = &go_green;
	}
}

void go_green() {
	if ( --pixels[0] == 0 ) {
		animator = &go_cyan;
	}
}

void go_cyan() {
	if ( ++pixels[2] == 255 ) {
		animator = &go_blue;
	}
}

void go_blue() {
	if ( --pixels[1] == 0 ) {
		animator = &go_magenta;
	}
}

void go_magenta() {
	if ( ++pixels[0] == 255 ) {
		animator = &go_red;
	}
}

void go_red() {
	if ( --pixels[2] == 0 ) {
		animator = &go_yellow;
	}

}

void init_animation() {
	for ( int i = 0 ; i < 5 * 6 * 3 ; ++i ) {
		pixels[i] = 0;
	}
	pixels[0] = 255;
	animator = &go_yellow;
}

void animate() {
	for ( int i = 1; i < 5 * 6; ++i) {
		int j = i - 1;
		pixels[3*i] = pixels[3*j];
		pixels[3*i+1] = pixels[3*j+1];
		pixels[3*i+2] = pixels[3*j+2];
	}
	(*animator)();

	for ( size_t x = 0; x < 5; ++x ) {
		for (size_t y = 0; y < 6; ++y) {
			size_t i = y * 5 + x;
			display_set_pixel(x,y,
							  pixels[3*i+0],pixels[3*i+1],pixels[3*i+2]);
		}
	}

}

int main() {
	clock_init();
	//init_animation();
	uint64_t period = 1000e3;
	uint64_t last = time_us_64();

	display_init();
	int i = 0;

#define is_set(bit) ( ( i & ( 1 << (bit) ) ) != 0 )
	while(true) {
		uint64_t now = time_us_64();
		if ( (now - last) >= period ) {
			last += period;
			++i;
			display_set_pixel(0,0,
							  is_set(0) * 255, is_set(0) * 255,is_set(0) * 255);
			display_set_pixel(0,1,
							  is_set(1) * 255, is_set(1) * 255,is_set(1) * 255);
			display_set_pixel(0,2,
							  is_set(2) * 255, is_set(2) * 255,is_set(2) * 255);
			display_set_pixel(0,3,
							  is_set(3) * 255, is_set(3) * 255,is_set(3) * 255);
			display_set_pixel(0,4,
							  is_set(4) * 255, is_set(4) * 255,is_set(4) * 255);
			display_set_pixel(0,5,
							  is_set(5) * 255, is_set(5) * 255,is_set(5) * 255);
		}
		display_process();

	}
	return 0;
}
