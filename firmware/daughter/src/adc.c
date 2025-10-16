#include "adc.h"

#include <avr/io.h>
#include <util/atomic.h>

#define CH0_MUXPOS_gc ADC_MUXPOS_AIN6_gc
#define CH1_MUXPOS_gc ADC_MUXPOS_AIN7_gc
#define CH2_MUXPOS_gc ADC_MUXPOS_AIN8_gc
#define CH3_MUXPOS_gc ADC_MUXPOS_AIN9_gc
#define CH4_MUXPOS_gc ADC_MUXPOS_AIN10_gc

static volatile uint8_t  next = 5;
static volatile uint16_t results[5];

void ADC_init() {

	// Enable interrupt for conversion ready
	ADC1.INTCTRL = ADC_RESRDY_bm;

	// No delay, but ensure random phase of the ADC conversion
	ADC1.CTRLD = ADC_INITDLY_DLY0_gc | ADC_ASDV_ASVON_gc;

	// Init a 625kHz clock from a 10MHz / 16 systemclock
	ADC1.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV16_gc;

	// 32 time subsampling. We get a sample every ms
	// or so, we get an update for a fader every 5 ms,
	// so we ensure 100Hz update.

	ADC1.CTRLB = ADC_SAMPNUM_ACC32_gc;

	// no free running.
	ADC1.CTRLA = ADC_RESSEL_10BIT_gc | ADC_ENABLE_bm;
}

inline static void start_next_conversion() {
	if (next >= 5) {
		return;
	}
	static uint8_t mux_values[5] = {
	    CH0_MUXPOS_gc,
	    CH1_MUXPOS_gc,
	    CH2_MUXPOS_gc,
	    CH3_MUXPOS_gc,
	    CH4_MUXPOS_gc,
	};

	ADC1.MUXPOS  = mux_values[next];
	ADC1.COMMAND = ADC_STCONV_bm;
}

ISR(ADC1_RESRDY_vect) {
	if (next < 5) {
		// take 1/32th of value, also clear the interupt flag
		results[next++] = (ADC1.RES) >> 5;
	} else {
		// we need to clear the flag. SHould not have happened
		ADC1.INTFLAGS |= ADC_RESRDY_bm;
	}
	start_next_conversion();
}

void ADC_start() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (next < 5) {
			// we are started, so do nothing
			return;
		}
		next = 0;
		start_next_conversion();
	}
}

uint16_t ADC_result(uint8_t index) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (next > index) {
			return results[index];
		}
	}
	return ADC_RESULT_NOT_READY;
}
