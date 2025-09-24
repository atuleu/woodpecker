#pragma once

#include "debouncer.h"
#include <stdint.h>

struct Encoder {
	uint8_t _initState;
	uint8_t _state;
	uint8_t value;

	struct Debouncer _qA, _qB;
};

void Encoder_init(struct Encoder *enc);
void Encoder_update(struct Encoder *enc, bool qA, bool qB);
