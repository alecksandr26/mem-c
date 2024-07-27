#ifndef CHK_H
#define CHK_H

#include <inttypes.h>
#include <except.h>

#include "heap.h"

typedef struct {
	const uint8_t *raddr;	/* reverserd address for user */
	uint8_t *ptr, *end;
	uint32_t size, capacity;
} Chk_T;

#define CHKPTR_PTR(chkptr) ((uint8_t *) (chkptr))
#define CHKPTR_RADDR(chkptr) (CHKPTR_PTR(chkptr) + sizeof(uint64_t))
#define CHKPTR_CAPACITY(chkptr) (*((uint64_t *) (CHKPTR_PTR(chkptr))))
#define CHKPTR_SIZE(chkptr) (CHKPTR_CAPACITY((chkptr)) + sizeof(uint64_t))
#define CHKPTR_END(chkptr) (CHKPTR_PTR(chkptr) + CHKPTR_SIZE(chkptr))
#define CHKPTR_FETCH_CHK_T(chkptr){ .raddr = CHKPTR_RADDR(chkptr),	\
			.ptr = CHKPTR_PTR(chkptr),	\
			.size = CHKPTR_SIZE(chkptr),			\
			.capacity = CHKPTR_CAPACITY((chkptr)),		\
			.end = CHKPTR_END((chkptr))			\
			}

extern Except_T ExceptOverFreededChunks;
extern unsigned int CHK_MIN_CHUNK_SIZE;
extern Heap_T heap_free_chunks;
extern int Chk_capacity_cmp(const void **chk1, const void **chk2);

#endif



