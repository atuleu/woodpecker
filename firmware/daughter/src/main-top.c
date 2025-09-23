#include "clock.h"
#include "uart.h"
#include <avr/io.h>
#include <stdio.h>

void initPins() {}

void init() {
	init_10MHz_clock();
	init_RTC_1kHz();
	initPins();
}

static int uart_putchar(char c, FILE *stream) {
	UART_wait_free();
	if (c == '\n') {
		UART_putc('\r');
		UART_wait_free();
	}
	UART_putc(c);
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

#define PERIOD_ms 250U

int main() {
	init();
	stdout = &mystdout;

	uint8_t last           = get_absolute_time() - PERIOD_ms;
	uint8_t quarter_second = 0;
	while (true) {
		UART_work();
		uint8_t now = get_absolute_time();
		if ((now - last) < PERIOD_ms) {
			continue;
		}
		last += PERIOD_ms;

		if ((quarter_second++ & 0x03) == 0) {
			printf("It is %d\n", quarter_second >> 2);
		}
	}
}
