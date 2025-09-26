#pragma once

#include <lwip/err.h>

#define OSC_PORT_IN  8000
#define OSC_PORT_OUT 9000

enum OSC_type {
	OSC_INT32 = 0,
	OSC_FLOAT32,
	OSC_STRING,
	OSC_RGBA,
	OSC_TRUE,
	OSC_FALSE,
};

#define OSC_TYPE_UNKNOWN (OSC_FALSE + 1)
typedef enum OSC_type OSC_type_e;

struct OSC_argument {
	OSC_type_e type;

	union {
		int32_t integer;
		float   float32;
		char   *string;
	} data;
};

typedef struct OSC_argument OSC_argument_t;

void OSC_argument_free(OSC_argument_t *arg);

struct OSC_message {
	char	      *address;
	OSC_argument_t argument;
};

typedef struct OSC_message OSC_message_t;

err_t osc_init();
void osc_deinit();
err_t osc_send(const OSC_message_t *m);

typedef void (*osc_recv_fn)(void *arg, const OSC_message_t *m);
void osc_recv(osc_recv_fn recv_fn, void *recv_arg);
