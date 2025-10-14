#pragma once

#include <hardware/sync.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
struct _saved_interrupt_block {
	uint32_t saved;
	bool     once;
};

static inline void
_restore_saved_interrupt(const struct _saved_interrupt_block *b) {
	restore_interrupts(b->saved);
}

#define ATOMIC_CORE_BLOCK()                                                    \
	for (struct _saved_interrupt_block                                         \
	     __attribute__((__cleanup__(_restore_saved_interrupt)))                \
	     saved = {.saved = save_and_disable_interrupts(), .once = true};       \
	     saved.once;                                                           \
	     saved.once = false)

#ifdef __cplusplus
}
#endif
