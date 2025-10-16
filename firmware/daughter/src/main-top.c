#include <avr/io.h>
#include <util/delay.h>

#include "clock.h"
#include "debouncer.h"
#include "encoder.h"
#include "frame.h"
#include "uart.h"

#define ADDR0_bm _BV(5)
#define ADDR1_bm _BV(6)

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

#define ALL_COL_bm (COL1_bm | COL2_bm | COL3_bm | COL4_bm | COL5_bm)

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

void init() {
	init_10MHz_clock();
	init_RTC_1kHz();
	initPins();
	init_UART(true);
	for (unsigned int i = 0; i < 10; ++i) {
		Encoder_init(&encoders[i]);
		Debouncer_init(&buttons[i]);
	}
	_delay_us(20);
	frame.ID = ((uint8_t)((PORTA.IN & ADDR1_bm) != 0) << 1) |
	           ((PORTA.IN & ADDR0_bm) != 0);
	frame.Type       = ENCODER;
	frame.SequenceID = 0x07;
}

uint8_t read_row(uint8_t row_mask) {
	PORTB.OUTCLR = row_mask;
	_delay_us(20);
	uint8_t res  = PORTC.IN & ALL_COL_bm;
	PORTB.OUTSET = row_mask;
	return res;
}

static inline void
update_group(uint8_t offset, uint8_t qA_bm, uint8_t qB_bm, uint8_t sw_bm) {
	uint8_t qA = read_row(qA_bm);
	uint8_t qB = read_row(qB_bm);
	uint8_t sw = read_row(sw_bm);

	for (uint8_t i = offset; i < offset + 5; ++i) {
		uint8_t col_bm = _BV(i - offset);
		Encoder_update(&encoders[i], (qA & col_bm) != 0, (qB & col_bm) != 0);
		Frame_set_encoder(&frame, i, encoders[i].value >> 2);
		// Frame_set_encoder(&frame, i, 0);
		Debouncer_5ms_push(&buttons[i], (sw & col_bm) != 0);
	}
}

static inline void write_buttons_to_frame() {
	uint16_t buttonValues = 0;
	for (uint8_t i = 0; i < 10; ++i) {
		buttonValues |= (uint16_t)buttons[i].state << i;
	}
	frame.Buttons = buttonValues;
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

		// UART_putc(now);

		update_group(0, R1_QA_bm, R1_QB_bm, R2_SW_bm);
		update_group(5, R3_QA_bm, R3_QB_bm, R4_SW_bm);
		write_buttons_to_frame();

		frame.SequenceID += 1;
		UART_push_frame(&frame);
	}
}
