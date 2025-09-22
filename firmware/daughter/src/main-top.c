#include "clock.h"
#include <avr/io.h>

void initPins() {}

void init() {
	init_10MHz_clock();
	initPins();
}

int main() {
	init();
}
