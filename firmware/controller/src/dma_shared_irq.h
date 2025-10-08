#pragma once

#include <pico/types.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*dma_channel_irq_handler_fn)(void *context, int channel);

void register_dma_channel_handler(
    uint irq_num, int channel, dma_channel_irq_handler_fn handler, void *context
);

void unregister_dma_channel_handler(uint irq_num, int channel);

#ifdef __cplusplus
}
#endif
