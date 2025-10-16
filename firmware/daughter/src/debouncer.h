#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Debouncer {
	bool    state;
	uint8_t _readSequence;
};

void Debouncer_init(struct Debouncer *debouncer);
bool Debouncer_5ms_push(struct Debouncer *debouncer, bool value);
bool Debouncer_2ms_push(struct Debouncer *debouncer, bool value);
