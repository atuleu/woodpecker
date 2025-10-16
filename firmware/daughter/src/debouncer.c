#include "debouncer.h"

#define DEBOUNCE_MASK(ms)        (((uint8_t)1 << ms) - 1)
#define DEBOUNCE_TRUE_VALUE(ms)  DEBOUNCE_MASK(ms)
#define DEBOUNCE_FALSE_VALUE(ms) 0x00

void Debouncer_init(struct Debouncer *debouncer) {
	debouncer->state         = false;
	debouncer->_readSequence = 0x00;
}

#define IMPLEMENT_DEBOUNCER_PUSH(v)                                            \
	bool Debouncer_##v##ms_push(struct Debouncer *debouncer, bool value) {     \
		debouncer->_readSequence =                                             \
		    (debouncer->_readSequence << 1) | (value & 0x01);                  \
		uint8_t test = debouncer->_readSequence & DEBOUNCE_MASK(v);            \
		if (debouncer->state == true) {                                        \
			if (test == DEBOUNCE_FALSE_VALUE(v)) {                             \
				debouncer->state = false;                                      \
				return true;                                                   \
			}                                                                  \
			return false;                                                      \
		} else {                                                               \
			if (test == DEBOUNCE_TRUE_VALUE(v)) {                              \
				debouncer->state = true;                                       \
				return true;                                                   \
			}                                                                  \
			return false;                                                      \
		}                                                                      \
	}

IMPLEMENT_DEBOUNCER_PUSH(2)
IMPLEMENT_DEBOUNCER_PUSH(5)
