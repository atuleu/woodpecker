#include "encoder.h"
#include "debouncer.h"

void Encoder_init(struct Encoder *enc) {
	enc->value      = 0x00;
	enc->_initState = 0x03;
}

void Encoder_update(struct Encoder *enc, bool qA, bool qB) {
	if (enc->_initState != 0) {
		// we push value until we settle the initial state.
		enc->_initState &=
		    ~(((uint8_t)Debouncer_push(&enc->_qA, qA) << 1) |
		      ((uint8_t)Debouncer_push(&enc->_qB, qB) << 0));
		enc->_state =
		    ((uint8_t)enc->_qA.state << 1) | ((uint8_t)enc->_qB.state << 0);

		return;
	}

	Debouncer_push(&enc->_qA, qA);
	Debouncer_push(&enc->_qB, qB);

	enc->_state = (enc->_state << 2) | (((uint8_t)enc->_qA.state << 1) & 0x02) |
	              (((uint8_t)enc->_qB.state << 0) & 0x01);

	// clang-format off
	const static int8_t transition_table[16] = {
		+0 /*00*/,-1 /*01*/, -1 /*10*/, -0 /*11*/, // last is 00
		+1 /*00*/,+0 /*01*/, -0 /*10*/, -1 /*11*/, // last is 01
		-1 /*00*/,-0 /*01*/, +0 /*10*/, +1 /*11*/, // last is 10
		-0 /*00*/,+1 /*01*/, -1 /*10*/, +0 /*11*/, // last is 11
	};
	// clang-format on

	enc->value += transition_table[enc->_state & 0x0f];
}
