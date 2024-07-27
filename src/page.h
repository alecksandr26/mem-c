#ifndef PAGE_H
#define PAGE_H

#include <inttypes.h>
#include "heap.h"
#include "chk.h"

typedef struct {
	uint8_t *ptr, *available, *end;
	uint64_t size, capacity;
} Page_T;


#define PAGEPTR_START_ADDR(pageptr) ((uint8_t *) (pageptr))
#define PAGEPTR_AVAILABLE_ADDR(pageptr) ((uint8_t *) *((uint64_t *) (PAGEPTR_START_ADDR(pageptr))))
#define PAGEPTR_END_ADDR(pageptr) ((uint8_t *) *((uint64_t *) ((PAGEPTR_START_ADDR(pageptr)) + sizeof(uint64_t))))
#define PAGEPTR_SIZE(pageptr) ((uint64_t) (PAGEPTR_END_ADDR(pageptr) - PAGEPTR_START_ADDR(pageptr)))
#define PAGEPTR_CAPACITY(pageptr) ((uint64_t) (PAGEPTR_END_ADDR(pageptr) - PAGEPTR_AVAILABLE_ADDR(pageptr)))
#define PAGEPTR_FETCH_PAGE_T(pageptr) { .ptr = PAGEPTR_START_ADDR(pageptr), \
			.available = (uint8_t *) PAGEPTR_AVAILABLE_ADDR(pageptr), \
			.end = (uint8_t *) PAGEPTR_END_ADDR(pageptr), \
			.size = PAGEPTR_SIZE(pageptr),			\
			.capacity = PAGEPTR_CAPACITY(pageptr)		\
			}


extern Heap_T heap_pages;
extern Except_T ExceptFatalPageError;
extern int Page_capacity_cmp(const void **addr1, const void **addr2);
extern void Page_alloc(Page_T *page, uint64_t nbytes);
extern void Page_chk_alloc(Page_T *page, Chk_T *chk);
extern void Page_chk_free(Page_T *page, Chk_T *chk);
extern void Page_free(Page_T *page);

#endif



