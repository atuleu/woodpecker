#include "dma_shared_irq.h"
#include "atomic.h"
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/platform_defs.h>
#include <hardware/regs/intctrl.h>
#include <hardware/sync.h>
#include <string.h>

#if NUM_DMA_IRQS ==2 && NUM_DMA_CHANNELS == 12
static dma_channel_irq_handler_fn handlers[NUM_DMA_CHANNELS * NUM_DMA_IRQS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static void *contexts[NUM_DMA_CHANNELS * NUM_DMA_IRQS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#endif

#define DECLARE_IRQ_HANDLER(n)                                                 \
	void __isr __not_in_flash_func(dma_channel_irq##n##_handler)(void) {       \
		uint ints       = dma_hw->ints##n;                                     \
		dma_hw->ints##n = ints;                                                \
		for (uint i = 0; i < NUM_DMA_CHANNELS; ++i) {                          \
			dma_channel_irq_handler_fn fn =                                    \
			    handlers[i + NUM_DMA_CHANNELS * n];                            \
			if (fn != NULL && (ints & (1u << i)) != 0) {                       \
				fn(contexts[i + NUM_DMA_CHANNELS * n], i);                     \
			}                                                                  \
		}                                                                      \
	}

#if NUM_DMA_IRQS > 0
DECLARE_IRQ_HANDLER(0);
#endif
#if NUM_DMA_IRQS > 1
DECLARE_IRQ_HANDLER(1);
#endif

// for futur use... but not sure there will be more
#if NUM_DMA_IRQS > 2
DECLARE_IRQ_HANDLER(2);
#endif

#if NUM_DMA_IRQS > 3
DECLARE_IRQ_HANDLER(3);
#endif

void register_dma_channel_handler(
    uint irq_num, int channel, dma_channel_irq_handler_fn handler, void *context
) {
	assert(irq_num == DMA_IRQ_0 || irq_num == DMA_IRQ_1);
	assert(channel < NUM_DMA_CHANNELS);

	if (irq_num == DMA_IRQ_0) {
		irq_set_exclusive_handler(irq_num, dma_channel_irq0_handler);
		dma_channel_set_irq0_enabled(channel, true);
		ATOMIC_CORE_BLOCK() {
			contexts[channel] = context;
			handlers[channel] = handler;
		}
	}

	if (irq_num == DMA_IRQ_1) {
		irq_set_exclusive_handler(irq_num, dma_channel_irq1_handler);
		dma_channel_set_irq1_enabled(channel, true);
		ATOMIC_CORE_BLOCK() {
			contexts[channel + 1 * NUM_DMA_CHANNELS] = context;
			handlers[channel + 1 * NUM_DMA_CHANNELS] = handler;
		}
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
