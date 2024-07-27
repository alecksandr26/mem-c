#include <sys/mman.h>
#include <except.h>
#include <except/assert.h>
#include <string.h>
#include <errno.h>

#include "page.h"
#include "heap.h"
#include "chk.h"
#include "utils.h"

Heap_T heap_pages = {
	.size = 0
};

Except_T ExceptFatalPageError = INIT_EXCEPT_T("Fatal error in page manipulation");

int Page_capacity_cmp(const void **addr1, const void **addr2)
{
	const uint8_t *pageptr1 = (const uint8_t *) *addr1;
	const uint8_t *pageptr2 = (const uint8_t *) *addr2;

	Page_T page1 = PAGEPTR_FETCH_PAGE_T(pageptr1);
	Page_T page2 = PAGEPTR_FETCH_PAGE_T(pageptr2);

	return page1.capacity - page2.capacity;
}

void Page_alloc(Page_T *page, uint64_t nbytes)
{
	assert(page != NULL, "Can't be null");
	assert(nbytes > 0, "Can't allocate zero bytes");
	assert(nbytes % 8 == 0, "Except to be multiple of 8");

	nbytes = aling_to_mul_4kb(nbytes + 2 * sizeof(uint64_t));
	uint8_t *pageptr = mmap(NULL, nbytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (pageptr == MAP_FAILED)
		RAISE(ExceptFatalPageError, "mmap: %s", strerror(errno));

	*((uint64_t *) (PAGEPTR_START_ADDR(pageptr))) = (uint64_t) (pageptr + 2 * sizeof(uint64_t));
	*((uint64_t *) ((PAGEPTR_START_ADDR(pageptr)) + sizeof(uint64_t))) = (uint64_t) (pageptr + nbytes);

	
	Page_T p = PAGEPTR_FETCH_PAGE_T(pageptr);
	*page = p;


	assert(PAGEPTR_AVAILABLE_ADDR(pageptr) == p.available);
	assert(p.capacity >= nbytes - 2 * sizeof(uint64_t));
	assert(p.size >= nbytes);
	assert(p.available == pageptr + 2 * sizeof(uint64_t));
	assert(p.end == pageptr + nbytes);
	assert(p.ptr == pageptr);
}

void Page_chk_alloc(Page_T *page, Chk_T *chk)
{
	assert(page != NULL && page->available != NULL
	       && page->end != NULL && page->ptr != NULL, "Can't be null");
	assert(chk != NULL, "Can't be null");
	assert(chk->size > 0, "Can't allocate an empty chunk");
	assert(chk->size + sizeof(uint64_t) < page->size, "The size can't overpass the page size");
	assert(page->available + chk->size <= page->end, "The chunk needs to fit");
	assert(chk->size % 8 == 0, "Must be multiple of 8");
	assert(chk->size >= CHK_MIN_CHUNK_SIZE, "Must be gretaer or equal to the min Chunk size");
	

	chk->ptr = page->available;
	chk->end = chk->ptr + chk->size;
	chk->raddr = page->available + sizeof(uint64_t);
	chk->capacity = chk->size - sizeof(uint64_t);
	*((uint64_t *) chk->ptr) = chk->capacity;

#ifndef NDEBUG
	/* Just to verify that everything works */
	Chk_T c = CHKPTR_FETCH_CHK_T(chk->ptr);

	assert(c.capacity == chk->capacity);
	assert(c.size == chk->size);
	assert(c.ptr == chk->ptr);
	assert(c.raddr == chk->raddr);
	assert(c.raddr > page->available && c.raddr < page->end);
#endif

	page->available += chk->size;
	page->capacity -= chk->size;
	*((uint64_t *) (PAGEPTR_START_ADDR(page->ptr))) = (uint64_t) page->available;
	assert(PAGEPTR_AVAILABLE_ADDR(page->ptr) == page->available);
}

void Page_chk_free(Page_T *page, Chk_T *chk)
{
	assert(page != NULL && page->available != NULL
	       && page->end != NULL && page->ptr != NULL, "Can't be null");
	assert(chk != NULL && chk->raddr != NULL
	       && chk->ptr != NULL, "Can't be null");
	assert(chk->size > 0, "Can't free an empty chunk");
	assert(chk->capacity > 0, "Can't free an empty capacity chunk");
	assert(chk->size + sizeof(uint64_t) < page->size, "The size can't overpass the page size");
	assert(chk->size % 8 == 0, "Must be multiple of 8");
	assert(chk->size >= CHK_MIN_CHUNK_SIZE, "Must be gretaer or equal to the min Chunk size");

	/* TODO: Combine all continues freeded chunks */
	
	if (chk->ptr + chk->size == page->available) {
		page->available = chk->ptr;
		page->capacity += chk->size;
		*((uint64_t *) (PAGEPTR_START_ADDR(page->ptr))) = (uint64_t) page->available;
		assert(PAGEPTR_AVAILABLE_ADDR(page->ptr) == page->available);
		return; 
	}
	
	if (heap_free_chunks.size == HEAP_CAPACITY)
		RAISE(ExceptOverFreededChunks, "Heap of free chunks is overpassed");
	
	Heap_push(&heap_free_chunks, chk->ptr, &Chk_capacity_cmp);
}

void Page_free(Page_T *page)
{

	if (munmap(page->ptr, page->size) == -1)
		RAISE(ExceptFatalPageError, "munmap: %s", strerror(errno));
	
	bzero(page, sizeof(*page));
}



