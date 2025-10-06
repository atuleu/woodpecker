#pragma once

#include <hardware/dma.h>
#include <hardware/i2c.h>

#include <stdint.h>

typedef uint32_t i2c_dma_xmit_id;

typedef struct i2c_dma_inst i2c_dma_inst_t;

i2c_dma_inst_t *i2c_dma_init(i2c_inst_t *i2c);
void            i2c_dma_deinit(i2c_dma_inst_t *i2c_dma);

int i2c_dma_reserve_xmit(
    i2c_dma_inst_t *i2c_dma, uint8_t addr, size_t len, i2c_dma_xmit_id *xmit
);
int i2c_dma_commit_xmit(i2c_dma_inst_t *i2c_dma, i2c_dma_xmit_id xmit);

bool i2c_dma_xmit_done(const i2c_dma_inst_t *i2c_dma, i2c_dma_xmit_id xmit);

int i2c_dma_xmit_write(
    i2c_dma_inst_t *i2c_dma,
    i2c_dma_xmit_id xmit,
    size_t          offset,
    const uint8_t  *src,
    size_t          len,
    bool            nostop,
    bool            norestart
);

int i2c_dma_xmit_read(
    i2c_dma_inst_t *i2c_dma,
    i2c_dma_xmit_id xmit,
    size_t          offset,
    uint8_t        *dst,
    size_t          len,
    bool            nostop,
    bool            norestart
);
