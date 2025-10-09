#include "lp5864.h"
#include "i2c_dma.h"
#include <hardware/i2c.h>
#include <pico/error.h>
#include <stdint.h>

static uint8_t build_lp5864_addr(uint8_t device_addr, uint reg_addr) {
	return 0b100 << 4 | ((device_addr & 0x03) << 2) | ((reg_addr & 0x300) >> 8);
}

int lp5864_read_blocking(
    i2c_inst_t *i2c, uint8_t addr, uint reg_addr, uint8_t *dst, size_t len
) {
	// builds the actual I2C address from register and device address.
	addr             = build_lp5864_addr(addr, reg_addr);
	uint8_t reg_mask = reg_addr & 0xff;

	int res = i2c_write_blocking(i2c, addr, &reg_mask, 1, true);
	if (res != 1) {
		// nothing was written or error.
		return res;
	}
	return i2c_read_blocking(i2c, addr, dst, len, false);
}

int lp5864_write_blocking(
    i2c_inst_t *i2c, uint8_t addr, uint reg_addr, const uint8_t *src, size_t len
) {
	addr             = build_lp5864_addr(addr, reg_addr);
	uint8_t reg_mask = reg_addr & 0xff;
	int     res      = i2c_write_burst_blocking(i2c, addr, &reg_mask, 1);
	if (res != 1) {
		// nothing was written or error.
		return res;
	}
	return i2c_write_blocking(i2c, addr, src, len, false);
}

int lp5864_schedule_read(
    i2c_dma_t       *i2c_dma,
    uint8_t          addr,
    uint             reg_addr,
    uint8_t         *dst,
    size_t           len,
    i2c_dma_xmit_id *xmit
) {
	addr             = build_lp5864_addr(addr, reg_addr);
	uint8_t reg_mask = reg_addr & 0xff;

	int res = i2c_dma_reserve_xmit(i2c_dma, addr, len + 1, xmit);
	if (res != PICO_OK) {
		return res;
	}

	res = i2c_dma_xmit_write(i2c_dma, *xmit, 0, &reg_mask, 1, true, false);
	if (res != PICO_OK) {
		return res;
	}

	res = i2c_dma_xmit_read(i2c_dma, *xmit, 1, dst, len, false, false);
	if (res != PICO_OK) {
		return res;
	}
	return i2c_dma_commit_xmit(i2c_dma, *xmit);
}

int lp5864_schedule_write(
    i2c_dma_t       *i2c_dma,
    uint8_t          addr,
    uint             reg_addr,
    const uint8_t   *src,
    size_t           len,
    i2c_dma_xmit_id *xmit
) {
	addr             = build_lp5864_addr(addr, reg_addr);
	uint8_t reg_mask = reg_addr & 0xff;

	int res = i2c_dma_reserve_xmit(i2c_dma, addr, len + 1, xmit);
	if (res != PICO_OK) {
		return res;
	}

	res = i2c_dma_xmit_write(i2c_dma, *xmit, 0, &reg_mask, 1, true, false);
	if (res != PICO_OK) {
		return res;
	}
	res = i2c_dma_xmit_write(i2c_dma, *xmit, 1, src, len, false, true);
	if (res != PICO_OK) {
		return res;
	}
	return i2c_dma_commit_xmit(i2c_dma, *xmit);
}
