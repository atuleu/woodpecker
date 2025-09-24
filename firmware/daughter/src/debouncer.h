#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Debouncer {
	bool    state;
	uint8_t _readSequence;
};

void Debouncer_init(struct Debouncer *debouncer);
bool Debouncer_push(struct Debouncer *debouncer, bool value);
