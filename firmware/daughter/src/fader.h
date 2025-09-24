#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Fader {
	uint16_t _value;
	uint8_t  _rem;
};

void Fader_init(struct Fader *f);

uint8_t Fader_update(struct Fader *f, uint16_t value_10b);
