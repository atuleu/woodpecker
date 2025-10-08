#include "i2c_dma.h"
#include "dma_shared_irq.h"

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/sync.h>

#include <hardware/timer.h>
#include <pico/error.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COMMAND_SIZE 512
#define COMMAND_MASK (COMMAND_SIZE - 1)
#define XMIT_SIZE    16
#define XMIT_MASK    (XMIT_SIZE - 1)

struct i2c_dma_xmit {
	volatile uint8_t done;
	uint8_t          addr;
	uint8_t         *dst;
	uint16_t         offset;
	uint16_t         length;
	uint16_t         buffer_length;
	uint16_t         read_length;
};

typedef struct i2c_dma_xmit i2c_dma_xmit_t;

static uint64_t i2c_dma_xmit_duration_us(i2c_dma_xmit_t *xmit, uint baudrate) {
	return (uint64_t)xmit->length * 9000000 / (uint64_t)baudrate;
}

struct i2c_dma_inst {
	uint16_t            commands[COMMAND_SIZE];
	struct i2c_dma_xmit xmit[XMIT_SIZE];

	volatile uint16_t        command_head, command_tail;
	volatile i2c_dma_xmit_id xmit_head, xmit_tail, xmit_head_done;
	volatile absolute_time_t deadline;

	i2c_inst_t *i2c;
	int         baudrate, scl, sda;

	uint command_channel, read_channel;
};

void i2c_dma_start_next_xmit(i2c_dma_inst_t *i2c_dma);

inline static uint16_t i2c_dma_used_commands(i2c_dma_inst_t *i2c_dma) {
	return i2c_dma->command_tail - i2c_dma->command_head;
}

inline static uint16_t i2c_dma_available_commands(i2c_dma_inst_t *i2c_dma) {
	return COMMAND_MASK - i2c_dma_used_commands(i2c_dma);
}

inline static bool i2c_dma_xmit_empty(i2c_dma_inst_t *i2c_dma) {
	return i2c_dma->xmit_head_done == i2c_dma->xmit_tail;
}

inline static bool i2c_dma_xmit_full(i2c_dma_inst_t *i2c_dma) {
	// This ring buffer use head==tail as empty. So we loose 1 capacity as we do
	// not keep a separate count.
	return (i2c_dma->xmit_tail - i2c_dma->xmit_head_done) >= (XMIT_SIZE - 1);
}

inline static void i2c_dma_xmit_command_done(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
		i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
		i2c_dma->xmit_head += 1;
		i2c_dma->command_head += xmit->buffer_length;
	}
}

inline static void i2c_dma_xmit_read_done(i2c_dma_inst_t *i2c_dma) {
	if (i2c_dma->xmit_head_done < i2c_dma->xmit_tail) {
		i2c_dma->xmit_head_done += 1;
	}
}

static i2c_dma_inst_t *context[NUM_DMA_CHANNELS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static inline void
i2c_dma_command_channel_irq_handler(i2c_dma_inst_t *i2c_dma, int channel) {
	(void)(channel);
	i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
	xmit->done += 1;
	if (xmit->done >= 1) {
		i2c_dma_xmit_command_done(i2c_dma);
		if (xmit->dst == NULL) {
			i2c_dma_xmit_read_done(i2c_dma);
		}
	}

	if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
		i2c_dma_start_next_xmit(i2c_dma);
	} else {
		i2c_dma->deadline = from_us_since_boot(-1);
	}
}

static inline void
i2c_dma_read_channel_irq_handler(i2c_dma_inst_t *i2c_dma, int channel) {
	(void)(channel);
	i2c_dma_xmit_read_done(i2c_dma);
}

i2c_dma_inst_t *i2c_dma_init(i2c_inst_t *i2c, int baudrate, int sda, int scl) {
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
		return NULL;
	}

	i2c_init(i2c, baudrate);
	res->baudrate = baudrate;
	res->scl      = scl;
	res->sda      = sda;

	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_set_function(sda, GPIO_FUNC_I2C);

	res->command_head   = 0;
	res->command_tail   = 0;
	res->xmit_head      = 1;
	res->xmit_tail      = 1;
	res->xmit_head_done = 1;

	register_dma_channel_handler(
	    DMA_IRQ_0,
	    res->command_channel,
	    (dma_channel_irq_handler_fn)i2c_dma_command_channel_irq_handler,
	    res
	);
	register_dma_channel_handler(
	    DMA_IRQ_0,
	    res->read_channel,
	    (dma_channel_irq_handler_fn)i2c_dma_read_channel_irq_handler,
	    res
	);
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
	dma_channel_set_read_addr(
	    res->read_channel,
	    &res->i2c->hw->data_cmd,
	    false
	);
	dma_channel_set_config(res->read_channel, &read_config, false);

	res->deadline = from_us_since_boot(-1);

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
	i2c_deinit(i2c_dma->i2c);
	free(i2c_dma);
}

int i2c_dma_reserve_xmit(
    i2c_dma_inst_t *i2c_dma, uint8_t addr, size_t len, i2c_dma_xmit_id *xmit
) {
	if (addr & 0x80) {
		// we only support 7bit addresses.
		return PICO_ERROR_INVALID_ARG;
	}

	uint32_t saved = save_and_disable_interrupts();
	size_t   available =
	    MAX(COMMAND_SIZE - (i2c_dma->command_tail & COMMAND_MASK),
	        (i2c_dma->command_head & COMMAND_MASK));

	if (available < len) {
		restore_interrupts(saved);
		printf(
		    "Cannot encode: %d, %d %d, high:%d low: %d.\n",
		    len,
		    i2c_dma->command_tail & COMMAND_MASK,
		    i2c_dma->command_head & COMMAND_MASK,
		    COMMAND_SIZE - (i2c_dma->command_tail & COMMAND_MASK),
		    (i2c_dma->command_head & COMMAND_MASK)
		);
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	if (i2c_dma_xmit_full(i2c_dma)) {
		restore_interrupts(saved);
		return PICO_ERROR_RESOURCE_IN_USE;
	}

	*xmit                = i2c_dma->xmit_tail;
	i2c_dma_xmit_t *next = &i2c_dma->xmit[i2c_dma->xmit_tail & XMIT_MASK];
	next->addr           = 0x7f & addr;
	next->length         = len;
	next->done           = 0;
	next->dst            = NULL;
	next->read_length    = 0;
	next->offset         = i2c_dma->command_tail & COMMAND_MASK;
	next->buffer_length  = len;
	if (next->offset + len > COMMAND_SIZE) {
		next->buffer_length += COMMAND_SIZE - next->offset;
		next->offset = 0;
	}

	i2c_dma->command_tail += next->buffer_length;

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
	if (next->dst != NULL && next->done == 0) {
		dma_channel_set_write_addr(i2c_dma->read_channel, next->dst, false);
		dma_channel_set_trans_count(
		    i2c_dma->read_channel,
		    dma_encode_transfer_count(next->read_length),
		    true
		);
	}

	dma_channel_set_read_addr(
	    i2c_dma->command_channel,
	    i2c_dma->commands + next->offset,
	    false
	);
	dma_channel_set_trans_count(
	    i2c_dma->command_channel,
	    dma_encode_transfer_count(next->length),
	    true
	);
	i2c_dma->deadline = make_timeout_time_us(
	    i2c_dma_xmit_duration_us(next, i2c_dma->baudrate) + 1000
	);
}

int i2c_dma_commit_xmit(i2c_dma_inst_t *i2c_dma, i2c_dma_xmit_id id) {
	uint32_t saved = save_and_disable_interrupts();
	bool     start = i2c_dma->xmit_tail == i2c_dma->xmit_head;
	if (id != i2c_dma->xmit_tail) {
		restore_interrupts(saved);
		return PICO_ERROR_INVALID_ARG;
	}

	i2c_dma->xmit_tail += 1;
	if (start == true) {
		i2c_dma_start_next_xmit(i2c_dma);
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

	if (offset + len > xmit->length) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	for (size_t i = 0; i < len; ++i) {
		size_t   j     = (offset + xmit->offset + i) & COMMAND_MASK;
		uint16_t value = src[i] & 0x00ff;
		if (i == 0 && norestart == false) {
			value |= I2C_IC_DATA_CMD_RESTART_BITS;
		}
		if (i == (len - 1) && nostop == false) {
			value |= I2C_IC_DATA_CMD_STOP_BITS;
		}
		i2c_dma->commands[j] = value;
	}
	return PICO_OK;
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

	if (offset + len > xmit->length) {
		return PICO_ERROR_INSUFFICIENT_RESOURCES;
	}

	xmit->dst         = dst;
	xmit->read_length = len;

	for (size_t i = 0; i < len; ++i) {
		size_t j = (offset + xmit->offset + i) & COMMAND_MASK;

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

void i2c_dma_i2c_unblock(i2c_dma_inst_t *i2c_dma);
bool i2c_dma_i2c_stucked(i2c_dma_inst_t *i2c_dma);

i2c_dma_xmit_id i2c_dma_check_and_failed_stalled(i2c_dma_inst_t *i2c_dma) {
	uint32_t saved = save_and_disable_interrupts();

	if (get_absolute_time() < i2c_dma->deadline ||
	    i2c_dma_xmit_empty(i2c_dma)) {
		restore_interrupts(saved);
		return 0;
	}
	i2c_dma_xmit_id xmit_id = i2c_dma->xmit_head_done;
	i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head_done & XMIT_MASK];

	dma_channel_abort(i2c_dma->command_channel);
	dma_channel_abort(i2c_dma->read_channel);
	// clear any misfired interrupt from disabled and know
	dma_hw->ints0 =
	    (1u << i2c_dma->command_channel) | (1u << i2c_dma->read_channel);

	// mark the xmit done.
	i2c_dma_xmit_command_done(i2c_dma);
	i2c_dma_xmit_read_done(i2c_dma);

	if (i2c_dma_i2c_stucked(i2c_dma) == true) {
		printf("[i2c_dma]: bus is stucked, unblocking it\n");
		i2c_dma_i2c_unblock(i2c_dma);
	}

	restore_interrupts(saved);
	// report an xmit has failed.
	return xmit_id;
}

#define pin_set_open_drain(pin)                                                \
	do {                                                                       \
		gpio_put(pin, false);                                                  \
		gpio_set_dir(pin, GPIO_IN);                                            \
		gpio_set_function(pin, GPIO_FUNC_SIO);                                 \
	} while (0)

#define pin_open_drain_high(pin)                                               \
	do {                                                                       \
		gpio_set_dir(pin, GPIO_IN);                                            \
	} while (0)

#define pin_open_drain_low(pin)                                                \
	do {                                                                       \
		gpio_set_dir(pin, GPIO_OUT);                                           \
	} while (0)

bool i2c_dma_i2c_stucked(i2c_dma_inst_t *i2c_dma) {
	pin_set_open_drain(i2c_dma->scl);
	pin_set_open_drain(i2c_dma->sda);

	bool res = gpio_get(i2c_dma->scl) == 0 || gpio_get(i2c_dma->sda) == 0;

	gpio_set_function(i2c_dma->scl, GPIO_FUNC_I2C);
	gpio_set_function(i2c_dma->sda, GPIO_FUNC_I2C);

	return res;
}

void i2c_dma_i2c_unblock(i2c_dma_inst_t *i2c_dma) {
	i2c_deinit(i2c_dma->i2c);

	pin_set_open_drain(i2c_dma->scl);
	pin_set_open_drain(i2c_dma->sda);

	// bit-banging a 100kHz clock burst until slave release its byte.
	for (uint i = 0; i < 9 && gpio_get(i2c_dma->sda) == 0; ++i) {
		pin_open_drain_low(i2c_dma->scl);
		busy_wait_until(make_timeout_time_us(5));
		pin_open_drain_high(i2c_dma->scl);
		busy_wait_until(make_timeout_time_us(5));
	}
	// bit-banging a STOP condition.
	pin_open_drain_low(i2c_dma->scl);
	busy_wait_until(make_timeout_time_us(5));
	pin_open_drain_low(i2c_dma->sda);
	busy_wait_until(make_timeout_time_us(5));
	pin_open_drain_high(i2c_dma->scl);
	busy_wait_until(make_timeout_time_us(5));
	pin_open_drain_high(i2c_dma->sda);
	busy_wait_until(make_timeout_time_us(5));

	gpio_set_function(i2c_dma->scl, GPIO_FUNC_I2C);
	gpio_set_function(i2c_dma->sda, GPIO_FUNC_I2C);
	i2c_init(i2c_dma->i2c, i2c_dma->baudrate);
}
