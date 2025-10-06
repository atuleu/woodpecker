#pragma once

#include <hardware/dma.h>
#include <hardware/i2c.h>

#include <stdint.h>

typedef struct i2c_dma_buffer i2c_dma_buffer_t;

typedef struct i2c_dma_inst i2c_dma_inst_t;

i2c_dma_inst_t *i2c_dma_init(i2c_inst_t *i2c);
void            i2c_dma_deinit(i2c_dma_inst_t *i2c_dma);

i2c_dma_buffer_t *i2c_dma_reserve(i2c_dma_inst_t *i2c_dma, size_t len);
void i2c_dma_commit(i2c_dma_inst_t *i2c_dma, const i2c_dma_buffer_t *buffer);

bool i2c_dma_buffer_done(const i2c_dma_buffer_t *buffer);

void i2c_dma_buffer_write(
    i2c_dma_buffer_t *buffer,
    size_t            offset,
    const uint8_t    *src,
    size_t            len,
    bool              nostop,
    bool              norestart
);

void i2c_dma_buffer_read(
    i2c_dma_buffer_t *buffer,
    size_t            offset,
    uint8_t          *dst,
    size_t            len,
    bool              nostop,
    bool              norestart
);
