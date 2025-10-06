#include "i2c_dma.h"

#include <hardware/dma.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/platform_defs.h>
#include <hardware/regs/i2c.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/dma.h>
#include <hardware/sync.h>
#include <stdint.h>
#include <stdlib.h>

#define RING_BUFFER_DECLARE(name, type, size)                                  \
	struct name##_rb {                                                         \
		type              data[size];                                          \
		volatile uint16_t head, tail;                                          \
		_Static_assert(                                                        \
		    size > 0 && (size & (size - 1)) == 0,                              \
		    "Size must be a power of two"                                      \
		);                                                                     \
	};                                                                         \
	typedef struct name##_rb name##_rb_t;                                      \
	void                     name##_rb_init(name##_rb_t *rb) {                 \
		                    rb->head = 0;                                      \
		                    rb->tail = 0;                                      \
	}                                                                          \
	static inline bool name##_rb_full(const name##_rb_t *rb) {                 \
		return ((rb->tail + 1) & (size - 1)) == rb->head;                      \
	}                                                                          \
	static inline bool name##_rb_empty(const name##_rb_t *rb) {                \
		return rb->head == rb->tail;                                           \
	}                                                                          \
	static inline uint16_t name##_rb_used(const name##_rb_t *rb) {             \
		if (rb->tail >= rb->head) {                                            \
			return rb->tail - rb->head;                                        \
		}                                                                      \
		return size + rb->tail - rb->head;                                     \
	}                                                                          \
	static inline uint16_t name##_rb_available(const name##_rb_t *rb) {        \
		return size - name##_rb_used(rb);                                      \
	}                                                                          \
	type *name##_rb_head(name##_rb_t *rb) {                                    \
		return &rb->data[rb->head];                                            \
	}                                                                          \
	type *name##_rb_tail(name##_rb_t *rb) {                                    \
		return &rb->data[rb->tail];                                            \
	}                                                                          \
	void name##_rb_push(name##_rb_t *rb, uint16_t n) {                         \
		uint32_t saved = save_and_disable_interrupts();                        \
		if (name##_rb_available(rb) >= n) {                                    \
			rb->tail = (rb->tail + n) & (size - 1);                            \
		}                                                                      \
		restore_interrupts(saved);                                             \
	}                                                                          \
	void name##_rb_pop(name##_rb_t *rb, uint16_t n) {                          \
		uint32_t saved = save_and_disable_interrupts();                        \
		if (name##_rb_used(rb) >= n) {                                         \
			rb->head = (rb->head + n) & (size - 1);                            \
		}                                                                      \
		restore_interrupts(saved);                                             \
	}

#define COMMAND_SIZE     512
#define TRANSACTION_SIZE 16

struct i2c_dma_buffer {
	volatile uint8_t done;
	uint8_t          length_transactions;
	uint16_t        *command_buffer;
	uint16_t         offset;
	uint16_t         length;
	uint16_t         length_read;
	uint8_t         *dst;
};

RING_BUFFER_DECLARE(i2c_command, uint16_t, COMMAND_SIZE)
RING_BUFFER_DECLARE(i2c_xmit, i2c_dma_buffer_t, TRANSACTION_SIZE)

void i2c_dma_irq_handler() {}

struct i2c_dma_inst {
	i2c_command_rb_t   commands;
	i2c_xmit_rb_t      transactions;
	i2c_inst_t        *i2c;
	uint               command_channel, read_channel;
	dma_channel_config command_config, read_config;
};

static i2c_dma_inst_t *context[NUM_DMA_CHANNELS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

i2c_dma_inst_t *i2c_dma_init(i2c_inst_t *i2c) {
	i2c_dma_inst_t *res = malloc(sizeof(i2c_dma_inst_t));
	if (res == NULL) {
		return res;
	}

	res->i2c             = i2c;
	res->command_channel = dma_claim_unused_channel(false);
	if (res->command_channel < 0) {
		free(res);
		return NULL;
	}
	res->read_channel = dma_claim_unused_channel(false);
	if (res->read_channel < 0) {
		dma_channel_unclaim(res->command_channel);
		free(res);
	}

	i2c_command_rb_init(&res->commands);
	i2c_xmit_rb_init(&res->transactions);

	irq_set_exclusive_handler(DMA_IRQ_0, i2c_dma_irq_handler);
	dma_channel_set_irq0_enabled(res->command_channel, true);
	dma_channel_set_irq0_enabled(res->read_channel, true);
	context[res->command_channel] = res;
	context[res->read_channel]    = res;
	// TODO: setup handler;

	res->command_config = dma_channel_get_default_config(res->command_channel);
	channel_config_set_read_increment(&res->command_config, true);
	channel_config_set_write_increment(&res->command_config, false);
	channel_config_set_transfer_data_size(&res->command_config, DMA_SIZE_16);
	channel_config_set_dreq(&res->command_config, i2c_get_dreq(res->i2c, true));

	res->read_config = dma_channel_get_default_config(res->read_channel);
	channel_config_set_read_increment(&res->read_config, false);
	channel_config_set_write_increment(&res->read_config, true);
	channel_config_set_transfer_data_size(&res->read_config, DMA_SIZE_8);
	channel_config_set_dreq(&res->read_config, i2c_get_dreq(res->i2c, false));

	return res;
}

void i2c_dma_deinit(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->command_channel >= 0) {
		dma_channel_unclaim(i2c_dma->command_channel);
	}
	if (i2c_dma->read_channel >= 0) {
		dma_channel_unclaim(i2c_dma->read_channel);
	}
	free(i2c_dma);
}

i2c_dma_buffer_t *i2c_dma_reserve(i2c_dma_inst_t *i2c_dma, size_t len) {
	uint32_t saved = save_and_disable_interrupts();
	if (i2c_command_rb_available(&i2c_dma->commands) < len ||
	    i2c_xmit_rb_full(&i2c_dma->transactions)) {
		restore_interrupts(saved);
		return NULL;
	}
	i2c_dma_buffer_t *next = i2c_xmit_rb_tail(&i2c_dma->transactions);
	next->command_buffer   = i2c_dma->commands.data;
	next->offset           = i2c_dma->commands.tail;
	next->done             = 0;
	next->length_read      = 0;
	if (next->offset + len >= COMMAND_SIZE) {
		next->length_transactions = 2;
	} else {
		next->length_transactions = 1;
	}
	i2c_command_rb_push(&i2c_dma->commands, len);
	restore_interrupts(saved);
	return next;
}

void i2c_dma_start_transactions(
    i2c_dma_inst_t *i2c_dma, const i2c_dma_buffer_t *buffer
) {

	if (buffer->length_read > 0 && buffer->done == 0) {
		dma_channel_configure(
		    i2c_dma->read_channel,
		    &i2c_dma->read_config,
		    buffer->dst,
		    &i2c_dma->i2c->hw->data_cmd,
		    buffer->length_read,
		    true
		);
	}

	if (buffer->offset + buffer->length < COMMAND_SIZE) {
		dma_channel_configure(
		    i2c_dma->command_channel,
		    &i2c_dma->command_config,
		    &i2c_dma->i2c->hw->data_cmd,
		    buffer->command_buffer + buffer->offset,
		    buffer->length,
		    true
		);
	} else if (buffer->done == 0) {
		dma_channel_configure(
		    i2c_dma->command_channel,
		    &i2c_dma->command_config,
		    &i2c_dma->i2c->hw->data_cmd,
		    buffer->command_buffer + buffer->offset,
		    COMMAND_SIZE - buffer->offset,
		    true
		);
	} else {
		dma_channel_configure(
		    i2c_dma->command_channel,
		    &i2c_dma->command_config,
		    &i2c_dma->i2c->hw->data_cmd,
		    buffer->command_buffer,
		    buffer->length - COMMAND_SIZE + buffer->offset,
		    true
		);
	}
}

void i2c_dma_commit(i2c_dma_inst_t *i2c_dma, const i2c_dma_buffer_t *buffer) {
	uint32_t saved = save_and_disable_interrupts();
	bool     start = i2c_xmit_rb_empty(&i2c_dma->transactions);
	if (buffer == i2c_xmit_rb_tail(&i2c_dma->transactions)) {
		i2c_xmit_rb_push(&i2c_dma->transactions, 1);
	}
	if (start) {
		i2c_dma_start_transactions(i2c_dma, buffer);
	}
	restore_interrupts(saved);
}

bool i2c_dma_buffer_done(const i2c_dma_buffer_t *buffer) {
	uint32_t saved = save_and_disable_interrupts();
	bool     res   = buffer->done >= buffer->length_transactions;
	restore_interrupts(saved);
	return res;
}

void i2c_dma_buffer_write(
    i2c_dma_buffer_t *buffer,
    size_t            offset,
    const uint8_t    *src,
    size_t            len,
    bool              nostop,
    bool              norestart
) {
	size_t j = buffer->offset + offset - 1;
	for (size_t i = 0; i < len; ++i) {
		if (i + offset >= buffer->length) {
			break;
		}
		++j;
		if (j >= COMMAND_SIZE) {
			j -= COMMAND_SIZE;
		}

		buffer->command_buffer[j] = src[i];
		if (i == 0 && norestart == false) {
			buffer->command_buffer[j] |= I2C_IC_DATA_CMD_RESTART_BITS;
		}
		if (i == (len - 1) && nostop == false) {
			buffer->command_buffer[j] |= I2C_IC_DATA_CMD_STOP_BITS;
		}
	}
}

void i2c_dma_buffer_read(
    i2c_dma_buffer_t *buffer,
    size_t            offset,
    uint8_t          *dst,
    size_t            len,
    bool              nostop,
    bool              norestart

) {
	buffer->dst = dst;
	size_t j    = buffer->offset + offset - 1;
	for (size_t i = 0; i < len; ++i) {
		if (i + offset >= buffer->length) {
			break;
		}
		++j;
		if (j >= COMMAND_SIZE) {
			j -= COMMAND_SIZE;
		}

		buffer->command_buffer[j] = I2C_IC_DATA_CMD_CMD_BITS;
		if (i == 0 && norestart == false) {
			buffer->command_buffer[j] |= I2C_IC_DATA_CMD_RESTART_BITS;
		}
		if (i == (len - 1) && nostop == false) {
			buffer->command_buffer[j] |= I2C_IC_DATA_CMD_STOP_BITS;
		}
		++buffer->length_read;
	}
}
