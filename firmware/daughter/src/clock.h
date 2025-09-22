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
