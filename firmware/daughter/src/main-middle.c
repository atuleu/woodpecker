#include "clock.h"

#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "clock.h"
#include "debouncer.h"
#include "encoder.h"
#include "fader.h"
#include "frame.h"
#include "uart.h"

#define ADDR0_bm _BV(5)
#define ADDR1_bm _BV(6)

#define COL1_bm _BV(0)
#define COL2_bm _BV(1)
#define COL3_bm _BV(2)
#define COL4_bm _BV(3)
#define COL5_bm _BV(4)

#define R5_SW_bp 5
#define R5_SW_bm _BV(5)
#define R6_SW_bp 6
#define R6_SW_bm _BV(6)

#define ALL_ROW_bm (R5_SW_bm | R6_SW_bm)

#define ALL_COL_bm (COL1_bm | COL2_bm | COL3_bm | COL4_bm | COL5_bm)

void initPins() {
	PORTA.DIRCLR = ADDR0_bm | ADDR1_bm;

	PORTB.DIRCLR = ALL_COL_bm;

	PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;

	PORTB.OUTSET = ALL_ROW_bm;
	PORTB.DIRSET = ALL_ROW_bm;
}

struct Debouncer buttons[10];
struct Fader     faders[5];

struct Frame frame;

void init() {
	init_10MHz_clock();
	init_RTC_1kHz();
	initPins();
	ADC_init();
	for (unsigned int i = 0; i < 10; ++i) {
		Debouncer_init(&buttons[i]);
		Fader_init(&faders[i / 2]);
	}
	_delay_us(20);
	frame.ID = ((uint8_t)((PORTA.IN & ADDR1_bm) != 0) << 1) |
	           ((PORTA.IN & ADDR0_bm) != 0);
	frame.Type       = FADER;
	frame.SequenceID = 0x07;
}

uint8_t read_row(uint8_t row_mask) {
	PORTB.OUTCLR = row_mask;
	_delay_us(20);
	uint8_t res  = PORTB.IN & ALL_COL_bm;
	PORTB.OUTSET = row_mask;
	return res;
}

void update_group(uint8_t offset, uint8_t sw_bm) {
	uint8_t sw = read_row(sw_bm);

	uint16_t buttonUpdate = 0;
	for (uint8_t i = offset; i < offset + 5; ++i) {
		uint8_t col_bm = _BV(i - offset);
		Debouncer_push(&buttons[i], (sw & col_bm) != 0);
	}
}

static inline void write_buttons_to_frame() {
	uint16_t buttonValues = 0;
	for (uint8_t i = 0; i < 10; ++i) {
		buttonValues |= (uint16_t)buttons[i].state << i;
	}
	frame.Buttons = buttonValues;
}

#define FADER_UPDATE_PERIOD_ms 10 // It is fine to only update fader at 100Hz

int main() {
	init();
	uint8_t lastButton = 255;
	uint8_t lastFader  = 255 - FADER_UPDATE_PERIOD_ms;
	uint8_t nextFader  = 5;
	while (true) {
		uint8_t now = get_absolute_time();
		if ((now - lastFader) >= FADER_UPDATE_PERIOD_ms) {
			lastFader += FADER_UPDATE_PERIOD_ms;
			// start a new read sequence, invalidate all results.
			ADC_start();
			nextFader = 0;
		}

		if (now == lastButton) {
			continue;
		}

		update_group(0, R5_SW_bm);
		update_group(5, R6_SW_bm);

		write_buttons_to_frame();

		// will return NOT_READY for invalid index.
		uint16_t result = ADC_result(nextFader);

		if (result != ADC_RESULT_NOT_READY) {
			frame.Fader[nextFader] = Fader_update(&faders[nextFader], result);
			++nextFader;
		}

		frame.SequenceID += 1;
		UART_push_frame(&frame);
	}
}
