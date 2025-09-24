#include "debouncer.h"

#define DEBOUNCE_MS          5
#define DEBOUNCE_MASK        (((uint8_t)1 << DEBOUNCE_MS) - 1)
#define DEBOUNCE_TRUE_VALUE  DEBOUNCE_MASK
#define DEBOUNCE_FALSE_VALUE 0x00

void Debouncer_init(struct Debouncer *debouncer) {
	debouncer->state         = false;
	debouncer->_readSequence = 0x00;
}

bool Debouncer_push(struct Debouncer *debouncer, bool value) {
	debouncer->_readSequence = (debouncer->_readSequence << 1) | (value & 0x01);
	uint8_t test             = debouncer->_readSequence & DEBOUNCE_MASK;
	if (debouncer->state == true) {
		if (test == DEBOUNCE_FALSE_VALUE) {
			debouncer->state = false;
			return true;
		}
		return false;
	} else {
		if (test == DEBOUNCE_TRUE_VALUE) {
			debouncer->state = true;
			return true;
		}
		return false;
	}
}
