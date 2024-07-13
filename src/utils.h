#ifndef UTILS_H
#define UTILS_H

#include <inttypes.h>

/* TODO: Create macro DBG_INF(msg, ...) */
/* TODO: Create macro DBG_WARN(msg, ...) */

#define CHUNK_SIZE(ptr) *((uint64_t *) ((ptr) - sizeof(uint64_t)))

/*
  ptr1 is greater than ptr2 -> 1 <= cmp
  ptr2 is greater than ptr1 -> -1 >= cmp
  ptr1 and ptr2 are equal -> 0 = cmp
 */
#define CHUNK_CMP(ptr1, ptr2) ((int64_t) ((CHUNK_SIZE(ptr1)) - (CHUNK_SIZE(ptr2))))

#endif
