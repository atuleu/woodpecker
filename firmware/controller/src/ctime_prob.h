#pragma once

#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctime_prob ctime_prob_t;

ctime_prob_t *ctime_prob_init(size_t windows_size, const char *name);
void          ctime_prob_free(ctime_prob_t *ctime);

void ctime_prob_push(ctime_prob_t *ctime, int64_t duration);

#ifdef __cplusplus
}
#endif
