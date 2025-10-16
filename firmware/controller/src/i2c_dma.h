#pragma once

#include <hardware/dma.h>
#include <hardware/i2c.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t i2c_dma_xmit_id;

typedef struct i2c_dma i2c_dma_t;

i2c_dma_t *i2c_dma_init(i2c_inst_t *i2c, int baudrate, int sda, int scl);
void       i2c_dma_deinit(i2c_dma_t *i2c_dma);

int i2c_dma_reserve_xmit(
    i2c_dma_t *i2c_dma, uint8_t addr, size_t len, i2c_dma_xmit_id *xmit
);
int i2c_dma_commit_xmit(i2c_dma_t *i2c_dma, i2c_dma_xmit_id xmit);

typedef uint8_t i2c_dma_xmit_status;

enum i2c_dma_xmit_status_e {
	I2C_DMA_XMIT_SCHEDULED = 0,
	I2C_DMA_XMIT_DONE      = 1 << 0,
	I2C_DMA_XMIT_ABORTED   = 1 << 1,
	I2C_DMA_XMIT_EXPIRED   = 1 << 2,
};

i2c_dma_xmit_status
i2c_dma_xmit_get_status(const i2c_dma_t *i2c_dma, i2c_dma_xmit_id xmit);

i2c_dma_xmit_status i2c_dma_xmit_wait(i2c_dma_t *i2c_dma, i2c_dma_xmit_id xmit);

i2c_dma_xmit_id i2c_dma_check_and_failed_stalled(i2c_dma_t *i2c_dma);

int i2c_dma_xmit_write(
    i2c_dma_t *i2c_dma,
    i2c_dma_xmit_id xmit,
    size_t          offset,
    const uint8_t  *src,
    size_t          len,
    bool            nostop,
    bool            norestart
);

int i2c_dma_xmit_read(
    i2c_dma_t *i2c_dma,
    i2c_dma_xmit_id xmit,
    size_t          offset,
    uint8_t        *dst,
    size_t          len,
    bool            nostop,
    bool            norestart
);

void i2c_dma_debugf(i2c_dma_t *i2c_dma);

#ifdef __cplusplus
}
#endif
