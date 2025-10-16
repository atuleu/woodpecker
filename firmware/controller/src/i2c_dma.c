#include "i2c_dma.h"
#include "atomic.h"
#include "dma_shared_irq.h"

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/platform_defs.h>
#include <hardware/regs/i2c.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/i2c.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/sync.h>

#include <hardware/timer.h>
#include <pico/error.h>
#include <pico/platform/compiler.h>
#include <pico/platform/sections.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COMMAND_SIZE 512
#define COMMAND_MASK (COMMAND_SIZE - 1)
#define XMIT_SIZE    16
#define XMIT_MASK    (XMIT_SIZE - 1)
#define STATUS_SIZE  64
#define STATUS_MASK  (STATUS_SIZE - 1)

#define static_assert_power_of_2(val)                                          \
	_Static_assert(                                                            \
	    val > 0 && (val & (val - 1)) == 0,                                     \
	    #val " must be a power of two"                                         \
	)
static_assert_power_of_2(COMMAND_SIZE);
static_assert_power_of_2(XMIT_SIZE);
static_assert_power_of_2(STATUS_SIZE);

struct i2c_dma_xmit {
	uint8_t  addr;
	uint8_t *dst;
	uint16_t offset;
	uint16_t length;
	uint16_t buffer_length;
	uint16_t read_length;
};

typedef struct i2c_dma_xmit i2c_dma_xmit_t;

static uint64_t i2c_dma_xmit_duration_us(i2c_dma_xmit_t *xmit, uint baudrate) {
	return (uint64_t)xmit->length * 9000000 / (uint64_t)baudrate;
}

struct i2c_dma {
	uint16_t            commands[COMMAND_SIZE];
	struct i2c_dma_xmit xmit[XMIT_SIZE];
	i2c_dma_xmit_status status[STATUS_SIZE];

	volatile uint16_t        command_head, command_tail;
	volatile i2c_dma_xmit_id xmit_head, xmit_tail;
	volatile absolute_time_t deadline;

	i2c_inst_t *i2c;
	int         baudrate, scl, sda;

	uint command_channel, read_channel;
};

void i2c_dma_start_next_xmit(i2c_dma_t *i2c_dma);

inline static uint16_t i2c_dma_used_commands(i2c_dma_t *i2c_dma) {
	return i2c_dma->command_tail - i2c_dma->command_head;
}

inline static uint16_t i2c_dma_available_commands(i2c_dma_t *i2c_dma) {
	return COMMAND_MASK - i2c_dma_used_commands(i2c_dma);
}

inline static bool i2c_dma_xmit_empty(i2c_dma_t *i2c_dma) {
	return i2c_dma->xmit_head == i2c_dma->xmit_tail;
}

inline static bool i2c_dma_xmit_full(i2c_dma_t *i2c_dma) {
	// This ring buffer use head==tail as empty. So we loose 1 capacity as we do
	// not keep a separate count.
	return (i2c_dma->xmit_tail - i2c_dma->xmit_head) >= (XMIT_SIZE - 1);
}

inline static void i2c_dma_xmit_mark_done(i2c_dma_t *i2c_dma) {
	if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
		i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
		i2c_dma->xmit_head += 1;
		i2c_dma->command_head += xmit->buffer_length;
		i2c_dma->deadline = 0x7fffffffffffffff;
	}
}

i2c_dma_t *contexts[NUM_I2CS] = {NULL, NULL};

static void i2c_dma_i2c_irq_handler(i2c_dma_t *i2c_dma) {

	i2c_hw_t	           *hw     = i2c_dma->i2c->hw;
	const volatile uint32_t status = hw->intr_stat;

	if (status & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
		hw->clr_tx_abrt;

		if (i2c_dma->xmit_head < i2c_dma->xmit_tail) {
			i2c_dma->status[i2c_dma->xmit_head & STATUS_MASK] =
			    I2C_DMA_XMIT_ABORTED;
			dma_channel_abort(i2c_dma->command_channel);
			dma_channel_abort(i2c_dma->read_channel);
		}
	}

	if (status & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
		hw->clr_stop_det;

		i2c_dma_xmit_mark_done(i2c_dma);

		if (i2c_dma->xmit_head == i2c_dma->xmit_tail) {
			i2c_dma->deadline = 0x7fffffffffffffffLL;
		} else {
			i2c_dma_start_next_xmit(i2c_dma);
		}
	}
}

static void __isr __not_in_flash_func(i2c_dma_i2c0_irq_handler)(void) {
	i2c_dma_i2c_irq_handler(contexts[0]);
}

static void __isr __not_in_flash_func(i2c_dma_i2c1_irq_handler)(void) {
	i2c_dma_i2c_irq_handler(contexts[1]);
}

i2c_dma_t *i2c_dma_init(i2c_inst_t *i2c, int baudrate, int sda, int scl) {
	i2c_dma_t *res = malloc(sizeof(i2c_dma_t));
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

	res->command_head = 0;
	res->command_tail = 0;
	res->xmit_head    = 1;
	res->xmit_tail    = 1;

	i2c->hw->intr_mask =
	    I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
	i2c->hw->clr_stop_det;
	i2c->hw->clr_tx_abrt;

	switch (i2c_get_index(i2c)) {
	case 0:
		ATOMIC_CORE_BLOCK() {
			contexts[0] = res;
			irq_set_exclusive_handler(I2C0_IRQ, i2c_dma_i2c0_irq_handler);
			irq_set_enabled(I2C0_IRQ, true);
		}
		break;
	case 1:
		ATOMIC_CORE_BLOCK() {
			contexts[1] = res;
			irq_set_exclusive_handler(I2C1_IRQ, i2c_dma_i2c1_irq_handler);
			irq_set_enabled(I2C1_IRQ, true);
		}
		break;
	default:
		dma_channel_unclaim(res->read_channel);
		dma_channel_unclaim(res->command_channel);
		i2c_deinit(i2c);
		free(res);
		return NULL;
	}

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

	res->deadline = 0x7fffffffffffffffLL;

	return res;
}

void i2c_dma_deinit(i2c_dma_t *i2c_dma) {
	i2c_deinit(i2c_dma->i2c);
	switch (i2c_get_index(i2c_dma->i2c)) {
	case 0:
		ATOMIC_CORE_BLOCK() {
			irq_set_enabled(I2C0_IRQ, false);
			contexts[0] = NULL;
			irq_remove_handler(I2C0_IRQ, i2c_dma_i2c0_irq_handler);
		}
		break;
	case 1:
		ATOMIC_CORE_BLOCK() {
			irq_set_enabled(I2C1_IRQ, false);
			contexts[0] = NULL;
			irq_remove_handler(I2C1_IRQ, i2c_dma_i2c1_irq_handler);
		}
		break;
	}
	if (i2c_dma->command_channel >= 0) {
		dma_channel_unclaim(i2c_dma->command_channel);
	}
	if (i2c_dma->read_channel >= 0) {
		dma_channel_unclaim(i2c_dma->read_channel);
	}
	free(i2c_dma);
}

int i2c_dma_reserve_xmit(
    i2c_dma_t *i2c_dma, uint8_t addr, size_t len, i2c_dma_xmit_id *xmit_id
) {
	if (addr & 0x80) {
		// we only support 7bit addresses.
		return PICO_ERROR_INVALID_ARG;
	}

	ATOMIC_CORE_BLOCK() {

		size_t available =
		    MAX(COMMAND_SIZE - (i2c_dma->command_tail & COMMAND_MASK),
		        (i2c_dma->command_head & COMMAND_MASK));

		if (available < len) {
			return PICO_ERROR_INSUFFICIENT_RESOURCES;
		}

		if (i2c_dma_xmit_full(i2c_dma)) {
			printf(
			    "[i2c_dma/%d] command:%s read:%s\n",
			    i2c_get_index(i2c_dma->i2c),
			    dma_channel_is_busy(i2c_dma->command_channel) ? "busy" : "free",
			    dma_channel_is_busy(i2c_dma->read_channel) ? "busy" : "free"
			);

			return PICO_ERROR_RESOURCE_IN_USE;
		}

		*xmit_id             = i2c_dma->xmit_tail;
		i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_tail & XMIT_MASK];
		xmit->addr           = 0x7f & addr;
		xmit->length         = len;
		xmit->dst            = NULL;
		xmit->read_length    = 0;
		xmit->offset         = i2c_dma->command_tail & COMMAND_MASK;
		xmit->buffer_length  = len;
		if (xmit->offset + len > COMMAND_SIZE) {
			xmit->buffer_length += COMMAND_SIZE - xmit->offset;
			xmit->offset = 0;
		}

		i2c_dma->command_tail += xmit->buffer_length;
	}
	return PICO_OK;
}

void i2c_dma_start_next_xmit(i2c_dma_t *i2c_dma) {
	if (i2c_dma->xmit_head >= i2c_dma->xmit_tail) {
		return;
	}

	i2c_dma_xmit_t *next     = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];
	i2c_dma->i2c->hw->enable = 0;
	i2c_dma->i2c->hw->tar    = next->addr;
	i2c_dma->i2c->hw->enable = 1;
	if (next->dst != NULL) {
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

int i2c_dma_commit_xmit(i2c_dma_t *i2c_dma, i2c_dma_xmit_id id) {
	ATOMIC_CORE_BLOCK() {
		bool start = i2c_dma->xmit_tail == i2c_dma->xmit_head;
		if (id != i2c_dma->xmit_tail) {
			return PICO_ERROR_INVALID_ARG;
		}

		i2c_dma->status[i2c_dma->xmit_tail & STATUS_MASK] =
		    I2C_DMA_XMIT_SCHEDULED;
		i2c_dma->xmit_tail += 1;
		if (start == true) {
			i2c_dma_start_next_xmit(i2c_dma);
		}
	}
	return PICO_OK;
}

i2c_dma_xmit_status
i2c_dma_xmit_get_status(const i2c_dma_t *i2c_dma, i2c_dma_xmit_id xmit) {
	i2c_dma_xmit_status res;
	ATOMIC_CORE_BLOCK() {

		if (i2c_dma->xmit_head <= xmit) {
			res = I2C_DMA_XMIT_SCHEDULED;
		} else if ((xmit + STATUS_SIZE) > i2c_dma->xmit_tail) {
			// we still have a valid status; need to check the validity of above
			// formula.
			res = i2c_dma->status[xmit & STATUS_MASK] | I2C_DMA_XMIT_DONE;
		} else {
			res = I2C_DMA_XMIT_DONE | I2C_DMA_XMIT_EXPIRED;
		}
	}
	return res;
}

int i2c_dma_xmit_write(
    i2c_dma_t      *i2c_dma,
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

i2c_dma_xmit_status
i2c_dma_xmit_wait(i2c_dma_t *i2c_dma, i2c_dma_xmit_id xmit) {
	i2c_dma_xmit_status res;
	do {
		res = i2c_dma_xmit_get_status(i2c_dma, xmit);
		i2c_dma_check_and_failed_stalled(i2c_dma);
	} while (res == I2C_DMA_XMIT_SCHEDULED);
	return res;
}

int i2c_dma_xmit_read(
    i2c_dma_t      *i2c_dma,
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

void i2c_dma_i2c_unblock(i2c_dma_t *i2c_dma);
bool i2c_dma_i2c_stucked(i2c_dma_t *i2c_dma);

i2c_dma_xmit_id i2c_dma_check_and_failed_stalled(i2c_dma_t *i2c_dma) {
	i2c_dma_xmit_id xmit_id;
	ATOMIC_CORE_BLOCK() {
		int64_t diff =
		    absolute_time_diff_us(i2c_dma->deadline, get_absolute_time());
		if (diff < 0 || i2c_dma_xmit_empty(i2c_dma)) {
			return 0;
		}
		printf(
		    "[i2c_dma/%d]: watchdog killing %u\n",
		    i2c_get_index(i2c_dma->i2c),
		    i2c_dma->xmit_head
		);

		xmit_id              = i2c_dma->xmit_head;
		i2c_dma_xmit_t *xmit = &i2c_dma->xmit[i2c_dma->xmit_head & XMIT_MASK];

		dma_hw->abort = 1U << i2c_dma->command_channel;
		dma_hw->abort = 1U << i2c_dma->read_channel;
		//  clear any misfired interrupt from disabled and know

		// mark the xmit done.
		i2c_dma_xmit_mark_done(i2c_dma);

		if (i2c_dma_i2c_stucked(i2c_dma) == true) {
			printf(
			    "[i2c_dma/%d]: bus is stucked, unblocking it\n",
			    i2c_get_index(i2c_dma->i2c)
			);
			i2c_dma_i2c_unblock(i2c_dma);
		}
		printf(
		    "[i2c_dma/%d]: killed %u\n",
		    i2c_get_index(i2c_dma->i2c),
		    i2c_dma->xmit_head - 1
		);
	}
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

bool i2c_dma_i2c_stucked(i2c_dma_t *i2c_dma) {
	pin_set_open_drain(i2c_dma->scl);
	pin_set_open_drain(i2c_dma->sda);

	bool res = gpio_get(i2c_dma->scl) == 0 || gpio_get(i2c_dma->sda) == 0;

	gpio_set_function(i2c_dma->scl, GPIO_FUNC_I2C);
	gpio_set_function(i2c_dma->sda, GPIO_FUNC_I2C);

	return res;
}

void i2c_dma_i2c_unblock(i2c_dma_t *i2c_dma) {
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

void i2c_dma_debugf(i2c_dma_t *i2c_dma) {
	ATOMIC_CORE_BLOCK() {
		printf(
		    "[i2c_dma/%d] head: %d tail: %d queued: %d status:[",
		    i2c_get_index(i2c_dma->i2c),
		    i2c_dma->xmit_head,
		    i2c_dma->xmit_tail,
		    i2c_dma->xmit_tail - i2c_dma->xmit_head
		);
		for (uint32_t i = i2c_dma->xmit_head; i < i2c_dma->xmit_tail; ++i) {
			printf(" %x", i2c_dma->status[i & STATUS_MASK]);
		}
		printf("]\n");
	}
}
