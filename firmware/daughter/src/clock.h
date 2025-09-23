#pragma once

#include <avr/io.h>

inline void init_10MHz_clock() {
	// enable clock change
	CCP               = CCP_IOREG_gc;
	// Sets a 10MHz clock for a 3.3 V power supply
	CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;
	// protect clock
	CCP               = 0;
}

inline void init_RTC_1kHz() {
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;
	RTC.CTRLA  = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm;
}

typedef uint8_t absolute_time_t;

#define get_absolute_time() (RTC.CNTL)
