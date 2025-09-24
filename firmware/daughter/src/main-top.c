#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>

#include "clock.h"
#include "debouncer.h"
#include "encoder.h"
#include "frame.h"
#include "uart.h"

#define ADDR0_bm _BV(5)
#define ADDR1_bm _BV(4)

#define COL1_bm _BV(0)
#define COL2_bm _BV(1)
#define COL3_bm _BV(2)
#define COL4_bm _BV(3)
#define COL5_bm _BV(4)

#define R4_SW_bp 2
#define R4_SW_bm _BV(2)
#define R3_QA_bp 3
#define R3_QA_bm _BV(3)
#define R3_QB_bp 4
#define R3_QB_bm _BV(4)
#define R2_SW_bp 5
#define R2_SW_bm _BV(5)
#define R1_QA_bp 6
#define R1_QA_bm _BV(6)
#define R1_QB_bp 7
#define R1_QB_bm _BV(7)

#define ALL_ROW_bm                                                             \
	(R4_SW_bm | R3_QA_bm | R3_QB_bm | R2_SW_bm | R1_QA_bm | R1_QB_bm)

void initPins() {
	PORTA.DIRCLR = ADDR0_bm | ADDR1_bm;

	PORTC.DIRCLR = COL1_bm | COL2_bm | COL3_bm | COL4_bm | COL5_bm;

	PORTC.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTC.PIN1CTRL = PORT_PULLUPEN_bm;
	PORTC.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTC.PIN3CTRL = PORT_PULLUPEN_bm;
	PORTC.PIN4CTRL = PORT_PULLUPEN_bm;

	PORTB.OUTSET = ALL_ROW_bm;
	PORTB.DIRSET = ALL_ROW_bm;
}

struct Encoder   encoders[10];
struct Debouncer buttons[10];

struct Frame frame;

uint8_t address;

void init() {
	init_10MHz_clock();
	init_RTC_1kHz();
	initPins();
	for (unsigned int i = 0; i < 10; ++i) {
		Encoder_init(&encoders[i]);
		Debouncer_init(&buttons[i]);
	}
	_delay_us(20);
	address = ((uint8_t)((PORTA.IN & ADDR1_bm) != 0) << 1) |
	          ((PORTA.IN & ADDR0_bm) != 0);
	frame.Type       = ENCODER;
	frame.ID         = address;
	frame.SequenceID = 0x07;
}

#define PERIOD_ms 250U
#define SCAN_DELAY_LOOP 18

uint8_t read_row(uint8_t row_mask) {
	PORTB.OUTCLR = row_mask;
	_delay_us(20);
	uint8_t res  = PORTC.IN & (COL1_bm | COL2_bm | COL3_bm | COL4_bm | COL5_bm);
	PORTB.OUTSET = row_mask;
	return res;
}

int main() {
	init();

	uint8_t last = 255;

	while (true) {
		uint8_t now = get_absolute_time();
		if (now == last) {
			continue;
		}
		last = now;

		uint8_t qa = read_row(R1_QA_bm);
		uint8_t qb = read_row(R1_QB_bm);
		uint8_t r  = read_row(R2_SW_bm);
		for (uint8_t i = 0; i < 5; ++i) {
			uint8_t mask = 1 << i;
			Encoder_update(&encoders[i], (qa & mask) != 0, (qb & mask) != 0);
			Frame_set_encoder(&frame, i, encoders[i].value);
			Debouncer_push(&buttons[i], (r & mask) != 0);
			Frame_set_button(&frame, i, buttons[i].state);
		}

		qa = read_row(R3_QA_bm);
		qb = read_row(R3_QB_bm);
		r  = read_row(R4_SW_bm);
		for (uint8_t i = 0; i < 5; ++i) {
			uint8_t mask = 1 << i;
			Encoder_update(
			    &encoders[i + 5],
			    (qa & mask) != 0,
			    (qb & mask) != 0
			);
			Frame_set_encoder(&frame, i + 5, encoders[i + 5].value);
			Debouncer_push(&buttons[i + 5], (r & mask) != 0);
			Frame_set_button(&frame, i + 5, buttons[i + 5].state);
		}
		frame.SequenceID += 1;
		UART_push_frame(&frame);
	}
}
