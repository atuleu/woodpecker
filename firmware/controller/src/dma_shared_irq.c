#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <string.h>

#include <hardware/dma.h>
#include <hardware/irq.h>

#include "atomic.h"
#include "dma_shared_irq.h"

#if NUM_DMA_IRQS == 2 && NUM_DMA_CHANNELS == 12
static dma_channel_irq_handler_fn handlers[NUM_DMA_CHANNELS * NUM_DMA_IRQS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static void *contexts[NUM_DMA_CHANNELS * NUM_DMA_IRQS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#endif

void __isr __not_in_flash_func(dma_channel_irq0_handler)(void) {
	uint ints = dma_hw->ints0;
	for (uint i = 0; i < NUM_DMA_CHANNELS; ++i) {
		dma_channel_irq_handler_fn fn = handlers[i];
		if (fn != NULL && (ints & (1u << i)) != 0) {
			fn(contexts[i], i);
		}
	}
	dma_hw->ints0 = ints;
}

void __isr __not_in_flash_func(dma_channel_irq1_handler)(void) {
	uint ints = dma_hw->ints1;
	for (uint i = 0; i < NUM_DMA_CHANNELS; ++i) {
		dma_channel_irq_handler_fn fn = handlers[i + NUM_DMA_CHANNELS];
		if (fn != NULL && (ints & (1u << i)) != 0) {
			fn(contexts[i + NUM_DMA_CHANNELS], i);
		}
	}
	dma_hw->ints1 = ints;
}

void register_dma_channel_handler(
    uint irq_num, int channel, dma_channel_irq_handler_fn handler, void *context
) {
	assert(irq_num == DMA_IRQ_0 || irq_num == DMA_IRQ_1);
	assert(channel < NUM_DMA_CHANNELS);
	if (irq_num == DMA_IRQ_0) {
		ATOMIC_CORE_BLOCK() {
			contexts[channel] = context;
			handlers[channel] = handler;
		}
		irq_set_exclusive_handler(DMA_IRQ_0, dma_channel_irq0_handler);
		dma_channel_set_irq0_enabled(channel, true);
	}

	if (irq_num == DMA_IRQ_1) {
		ATOMIC_CORE_BLOCK() {
			contexts[channel + NUM_DMA_CHANNELS] = context;
			handlers[channel + NUM_DMA_CHANNELS] = handler;
		}
		irq_set_exclusive_handler(DMA_IRQ_1, dma_channel_irq1_handler);
		dma_channel_set_irq1_enabled(channel, true);
	}

	irq_set_enabled(irq_num, true);
}

void unregister_dma_channel_handler(uint irq_num, int channel) {
	assert(irq_num == DMA_IRQ_0 || irq_num == DMA_IRQ_1);
	assert(channel < NUM_DMA_CHANNELS);
	ATOMIC_CORE_BLOCK() {
		if (irq_num == DMA_IRQ_0) {
			contexts[channel] = NULL;
			handlers[channel] = NULL;
		}

		if (irq_num == DMA_IRQ_1) {
			contexts[channel + NUM_DMA_CHANNELS] = NULL;
			handlers[channel + NUM_DMA_CHANNELS] = NULL;
		}
	}
}
