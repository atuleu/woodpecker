#include "i2c_dma.h"

#include <hardware/dma.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/sync.h>

#include <pico/error.h>
#include <stdint.h>
#include <stdlib.h>

#define COMMAND_SIZE 512
#define COMMAND_MASK (COMMAND_SIZE - 1)
#define XMIT_SIZE    16
#define XMIT_MASK    (XMIT_SIZE - 1)

struct i2c_dma_xmit {
	volatile uint8_t done;
	uint8_t          addr;
	uint16_t        *command_buffer;
	uint16_t         offset;
	uint16_t         length;
	uint8_t         *dst;
	size_t           read_length;
};

typedef struct i2c_dma_xmit i2c_dma_xmit_t;

#define i2c_xmit_size(xmit)                                                    \
	((((xmit)->offset + xmit->length) > COMMAND_SIZE) ? 2 : 1)

struct i2c_dma_inst {
	uint16_t            commands[COMMAND_SIZE];
	struct i2c_dma_xmit xmit[XMIT_SIZE];

	uint16_t        command_head, command_tail;
	i2c_dma_xmit_id xmit_head, xmit_tail, xmit_head_done;

	i2c_inst_t *i2c;
	uint        command_channel, read_channel;
};

void i2c_dma_start_next_xmit(i2c_dma_inst_t *i2c_dma);

inline static uint16_t i2c_dma_used_commands(i2c_dma_inst_t *i2c_dma) {
	return i2c_dma->command_tail - i2c_dma->command_head;
}

inline static uint16_t i2c_dma_available_commands(i2c_dma_inst_t *i2c_dma) {
	return COMMAND_SIZE - i2c_dma_used_commands(i2c_dma);
}

inline static bool i2c_dma_xmit_empty(i2c_dma_inst_t *i2c_dma) {
	return i2c_dma->xmit_head_done == i2c_dma->xmit_tail;
}

inline static bool i2c_dma_xmit_full(i2c_dma_inst_t *i2c_dma) {
	return (i2c_dma->xmit_tail - i2c_dma->xmit_head_done) >= XMIT_MASK;
}

inline static void i2c_dma_xmit_command_done(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
		i2c_dma->xmit_head += 1;
	}
}

inline static void i2c_dma_xmit_read_done(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->xmit_head_done < i2c_dma->xmit_tail) {
		i2c_dma_xmit_t *xmit =
		    &i2c_dma->xmit[i2c_dma->xmit_head_done & XMIT_MASK];
		i2c_dma->command_head += xmit->length;
		i2c_dma->xmit_head_done += 1;
	}
}

static i2c_dma_inst_t *context[NUM_DMA_CHANNELS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static inline void i2c_dma_command_channel_irq_handler(i2c_dma_inst_t *i2c_dma
) {
	i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
	xmit->done += 1;
	if (xmit->done == i2c_xmit_size(xmit)) {
		i2c_dma_xmit_command_done(i2c_dma);
		if (xmit->dst == NULL) {
			i2c_dma_xmit_read_done(i2c_dma);
		}
	}

	if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
		i2c_dma_start_next_xmit(i2c_dma);
	}
}

static inline void i2c_dma_read_channel_irq_handler(i2c_dma_inst_t *i2c_dma) {
	i2c_dma_xmit_read_done(i2c_dma);
}

static inline void
i2c_dma_channel_irq_handler(i2c_dma_inst_t *i2c_dma, uint channel) {
	if (channel == i2c_dma->command_channel) {
		i2c_dma_command_channel_irq_handler(i2c_dma);
	} else {
		i2c_dma_read_channel_irq_handler(i2c_dma);
	}
}

static void i2c_dma_irq_handler() {
	uint ints = dma_hw->ints0;
	for (uint8_t i = 0; i < 12; ++i) {
		uint mask = (1u << i);
		if (ints & mask) {
			if (context[i] != NULL) {
				i2c_dma_channel_irq_handler(context[i], i);
			}
		}
	}
	dma_hw->ints0 = ints;
}

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
	res->command_head   = 0;
	res->command_tail   = 0;
	res->xmit_head      = 0;
	res->xmit_tail      = 0;
	res->xmit_head_done = 0;

	irq_set_exclusive_handler(DMA_IRQ_0, i2c_dma_irq_handler);
	dma_channel_set_irq0_enabled(res->command_channel, true);
	dma_channel_set_irq0_enabled(res->read_channel, true);
	irq_set_enabled(DMA_IRQ_0, true);
	context[res->command_channel] = res;
	context[res->read_channel]    = res;

	dma_channel_config command_config =
	    dma_channel_get_default_config(res->command_channel);
	channel_config_set_read_increment(&command_config, true);
	channel_config_set_write_increment(&command_config, false);
	channel_config_set_transfer_data_size(&command_config, DMA_SIZE_16);
	channel_config_set_dreq(&command_config, i2c_get_dreq(res->i2c, true));
	dma_channel_set_write_addr(
	    res->command_channel,
	    &res->i2c->hw->data_cmd,
	    false
	);
	dma_channel_set_config(res->command_channel, &command_config, false);

	dma_channel_config read_config =
	    dma_channel_get_default_config(res->read_channel);
	channel_config_set_read_increment(&read_config, false);
	channel_config_set_write_increment(&read_config, true);
	channel_config_set_transfer_data_size(&read_config, DMA_SIZE_8);
	channel_config_set_dreq(&read_config, i2c_get_dreq(res->i2c, false));
	dma_channel_set_config(res->read_channel, &read_config, false);
	dma_channel_set_read_addr(
	    res->read_channel,
	    &res->i2c->hw->data_cmd,
	    false
	);

	return res;
}

void i2c_dma_deinit(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->command_channel >= 0) {
		dma_channel_unclaim(i2c_dma->command_channel);
		context[i2c_dma->command_channel] = NULL;
	}
	if (i2c_dma->read_channel >= 0) {
		dma_channel_unclaim(i2c_dma->read_channel);
		context[i2c_dma->read_channel] = NULL;
	}

	free(i2c_dma);
}

int i2c_dma_reserve_xmit(
    i2c_dma_inst_t *i2c_dma, uint8_t addr, size_t len, i2c_dma_xmit_id *xmit
) {
	uint32_t saved = save_and_disable_interrupts();
	if (i2c_dma_available_commands(i2c_dma) < len ||
	    i2c_dma_xmit_full(i2c_dma)) {
		restore_interrupts(saved);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	*xmit                = i2c_dma->xmit_tail;
	i2c_dma_xmit_t *next = &i2c_dma->xmit[i2c_dma->xmit_tail & XMIT_MASK];
	next->addr           = 0x7f & addr;
	next->offset         = i2c_dma->command_tail & COMMAND_MASK;
	next->done           = 0;
	next->dst            = NULL;
	next->length         = len;
	next->read_length    = 0;
	i2c_dma->command_tail += len;

	restore_interrupts(saved);
	return PICO_OK;
}

void i2c_dma_start_next_xmit(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->xmit_head >= i2c_dma->xmit_tail) {
		return;
	}

	i2c_dma_xmit_t *next     = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
	i2c_dma->i2c->hw->enable = 0;
	i2c_dma->i2c->hw->tar    = next->addr;
	i2c_dma->i2c->hw->enable = 1;
	if (next->dst > 0 && next->done == 0) {
		dma_channel_set_write_addr(i2c_dma->read_channel, next->dst, false);
		dma_channel_set_trans_count(
		    i2c_dma->read_channel,
		    next->read_length,
		    true
		);
	}

	if (next->offset + next->length < COMMAND_SIZE) {
		dma_channel_set_read_addr(
		    i2c_dma->command_channel,
		    i2c_dma->commands + next->offset,
		    false
		);
		dma_channel_set_trans_count(
		    i2c_dma->command_channel,
		    next->length,
		    true
		);
	} else if (next->done == 0) {
		dma_channel_set_read_addr(
		    i2c_dma->command_channel,
		    i2c_dma->commands + next->offset,
		    false
		);
		dma_channel_set_trans_count(
		    i2c_dma->command_channel,
		    COMMAND_SIZE - next->offset,
		    true
		);
	} else {
		dma_channel_set_read_addr(
		    i2c_dma->command_channel,
		    i2c_dma->commands,
		    false
		);
		dma_channel_set_trans_count(
		    i2c_dma->command_channel,
		    next->length - COMMAND_SIZE + next->offset,
		    true
		);
	}
}

int i2c_dma_commit_xmit(i2c_dma_inst_t *i2c_dma, i2c_dma_xmit_id id) {
	uint32_t saved = save_and_disable_interrupts();
	bool     start = i2c_dma->xmit_tail == i2c_dma->xmit_head;
	if (id != i2c_dma->xmit_tail) {
		restore_interrupts(saved);
		return PICO_ERROR_INVALID_ARG;
	}

	if (id == i2c_dma->xmit_tail) {
		i2c_dma->xmit_tail += 1;
		if (start) {
			i2c_dma_start_next_xmit(i2c_dma);
		}
	}
	restore_interrupts(saved);
	return PICO_OK;
}

bool i2c_dma_xmit_done(const i2c_dma_inst_t *i2c_dma, i2c_dma_xmit_id xmit) {
	uint32_t saved = save_and_disable_interrupts();
	bool     res   = i2c_dma->xmit_head_done > xmit;
	restore_interrupts(saved);
	return res;
}

int i2c_dma_xmit_write(
    i2c_dma_inst_t *i2c_dma,
    i2c_dma_xmit_id xmit_id,
    size_t          offset,
    const uint8_t  *src,
    size_t          len,
    bool            nostop,
    bool            norestart
) {
	i2c_dma_xmit_t *xmit = &i2c_dma->xmit[xmit_id & XMIT_MASK];

	size_t j = xmit->offset + offset - 1;
	for (size_t i = 0; i < len; ++i) {
		if (i + offset >= xmit->length) {
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		++j;
		if (j >= COMMAND_SIZE) {
			j -= COMMAND_SIZE;
		}

		uint16_t value = src[i];
		if (i == 0 && norestart == false) {
			value |= I2C_IC_DATA_CMD_RESTART_BITS;
		}
		if (i == (len - 1) && nostop == false) {
			value |= I2C_IC_DATA_CMD_STOP_BITS;
		}
		i2c_dma->commands[j] = value;
	}
}

int i2c_dma_xmit_read(
    i2c_dma_inst_t *i2c_dma,
    i2c_dma_xmit_id xmit_id,
    size_t          offset,
    uint8_t        *dst,
    size_t          len,
    bool            nostop,
    bool            norestart

) {
	i2c_dma_xmit_t *xmit = &i2c_dma->xmit[xmit_id & XMIT_MASK];

	xmit->dst         = dst;
	xmit->read_length = len;
	size_t j          = xmit->offset + offset - 1;
	for (size_t i = 0; i < len; ++i) {
		if (i + offset >= xmit->length) {
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}
		++j;
		if (j >= COMMAND_SIZE) {
			j -= COMMAND_SIZE;
		}

		uint16_t cmd = I2C_IC_DATA_CMD_CMD_BITS;
		if (i == 0 && norestart == false) {
			cmd |= I2C_IC_DATA_CMD_RESTART_BITS;
		}
		if (i == (len - 1) && nostop == false) {
			cmd |= I2C_IC_DATA_CMD_STOP_BITS;
		}
		i2c_dma->commands[j] = cmd;
	}
	return PICO_OK;
}
