#include "fader.h"

#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define CLAMP(v, low, high) MIN(MAX(v, low), high)

#define DEADBAND(v, high, ddb)                                                 \
	((CLAMP(v, ddb, high - ddb) - ddb) * (high - 2 * ddb) / high)

void Fader_init(struct Fader *f) {
	f->_value = 0xffff;
}

static inline uint8_t fader_map(uint16_t value10b) {
// this computes a 1% deadgap on both end at 8bit, keeping it an 16bit integer
// without overflow. We miss a few value, but who cares.
#define DEADGAP 3
	value10b = CLAMP(value10b >> 2, DEADGAP, 255 - DEADGAP);

	return ((value10b - DEADGAP) * 255) / (255 - DEADGAP - DEADGAP);
}

uint8_t Fader_update(struct Fader *f, uint16_t value_10b) {
	if (f->_value == 0xffff) {
		f->_value = MIN(value_10b, 1023);
		return fader_map(value_10b);
	}
	// add a bit of smoothing. At 200Hz sampling, this filter as a 270 Hz
	// cut-off so 27Hz signal are untouched. Should feel really responsive.
	f->_value = (f->_value + 3 * value_10b) >> 2;
	f->_value = MIN(f->_value, 1023);
	// we map it to 8bit.
	return fader_map(f->_value);
}
