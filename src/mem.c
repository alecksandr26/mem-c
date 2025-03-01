#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <except.h>
#include <except/assert.h>
#include <stddef.h>
#include <stdio.h>

#include "chk.h"
#include "utils.h"
#include "heap.h"
#include "page.h"

int default_log_fd = 1;
Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

void *mem_alloc(unsigned long nbytes)
{
	if (nbytes == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");

	// Align with the size of the metadata
	nbytes = aling_to_mul_8(nbytes + 2 * sizeof(uint64_t));
	nbytes = MAX((uint32_t) CHK_MIN_CHUNK_SIZE, nbytes);
	
	Chk_T chk = {
		.size = nbytes
	};

	if (heap_free_chunks.size > 0) {
		Chk_T top = CHKPTR_FETCH_CHK_T(Heap_top(&heap_free_chunks));
		Chk_combine_with_freeded_neighbor(&top, PAGEPTR_AVAILABLE_ADDR(top.pageptr));
	}
	
	if (heap_free_chunks.size > 0
	    && CHKPTR_SIZE(Heap_top(&heap_free_chunks)) >= nbytes) {
		uint8_t *frag_chkptr = Heap_pop(&heap_free_chunks, &Chk_capacity_cmp);
		Chk_T frag_chk = CHKPTR_FETCH_CHK_T(frag_chkptr);
		frag_chk.capacity -= chk.size;
		frag_chk.size -= chk.size;
		if (frag_chk.size >= CHK_MIN_CHUNK_SIZE) {
			/* TODO: Use the lower addressed chk and push to free the higher */
			chk.ptr = (uint8_t *) frag_chk.raddr + frag_chk.capacity;
			chk.capacity = chk.size - 2 * sizeof(uint64_t);
			chk.pageptr = frag_chk.pageptr;

			// Store the metadata for the new chunk
			*((uint64_t *) chk.ptr) = chk.capacity;
			*((uint64_t *) chk.ptr + 1) = (uint64_t) chk.pageptr;
			*((uint64_t *) frag_chk.ptr) = frag_chk.capacity;
			
			chk.raddr = chk.ptr + 2 * sizeof(uint64_t);
			
			assert(CHKPTR_CAPACITY(chk.ptr) == (uint32_t) chk.capacity);
			assert(CHKPTR_PAGEPTR(chk.ptr) == frag_chk.pageptr);
			assert(CHKPTR_CAPACITY(frag_chk.ptr) == (uint32_t) \
			       frag_chk.capacity);
			
			Chk_put_checksum(&frag_chk);
			Heap_push(&heap_free_chunks, frag_chk.ptr, &Chk_capacity_cmp);
			
			Chk_rem_checksum(&chk);
			/* Since it is a new chunk we need to map it */
			/* Trie_map((uint64_t) chk.raddr, Trie_find((uint64_t) frag_chk.raddr)); */
			return (void *) chk.raddr;
		}

		Chk_rem_checksum(&frag_chk);
		return (void *) frag_chk.raddr;
	}

	
	Page_T page;
	if (heap_pages.size > 0
	    && PAGEPTR_CAPACITY(Heap_top(&heap_pages)) > nbytes) {
		uint8_t *top_page_ptr = Heap_pop(&heap_pages, &Page_capacity_cmp);
		Page_T top_page = PAGEPTR_FETCH_PAGE_T(top_page_ptr);
		page = top_page;
	} else {
		Page_alloc(&page, nbytes);
	}

	Page_chk_alloc(&page, &chk);
	
	Heap_push(&heap_pages, page.ptr, &Page_capacity_cmp);
	assert(chk.capacity > 0 && chk.capacity < chk.size);
	assert(chk.ptr > page.ptr);
	assert(chk.raddr < page.end);
	assert(chk.raddr < page.available);
	assert(heap_pages.size > 0);

	return (void *) chk.raddr;
}


void mem_free(void *addr)
{
	uint8_t *chkptr = (uint8_t *) addr - 2 * sizeof(uint64_t);
	Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);

	uint8_t *pageptr = chk.pageptr;
	if (pageptr == NULL)
		RAISE(ExceptInvalidAddr, "Can't free an invalid addr");

	Page_T page = PAGEPTR_FETCH_PAGE_T(pageptr);
	if (chk.capacity == 0 || chk.size >= page.size)
		RAISE(ExceptCorruptedAddr, "The reserved addr has an invalid capacity");

	if (page.available <= chk.ptr
	    || Chk_verify_checksum(&chk) == 1)
		RAISE(ExceptInvalidAddr, "Address already freed");

	uint32_t page_index = Heap_find(&heap_pages, page.ptr,&Page_capacity_cmp);
	Heap_rem(&heap_pages, page_index, &Page_capacity_cmp);
	Page_chk_free(&page, &chk);

	if (page.available - 2 * sizeof(uint64_t) == page.ptr) {
		Page_free(&page);
		return;
	}

	Heap_push(&heap_pages, page.ptr, &Page_capacity_cmp);
}

void *mem_ralloc(void *addr, unsigned long nbytes)
{
	if (nbytes == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");
	
	uint8_t *chkptr = (uint8_t *) addr - sizeof(uint64_t);
	uint8_t *pageptr = Page_find_chks_page(addr);
	if (pageptr == NULL)
		RAISE(ExceptInvalidAddr, "Can't free an invalid addr");
	Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);
	
	void *new_addr = mem_alloc(nbytes);

	/* TODO: Implement a vectorized memcpy */
	uint32_t n = MIN(nbytes, (uint64_t) chk.capacity);
	memcpy(new_addr, addr, n);

	mem_free(addr);
	
	return new_addr;
}

void *mem_calloc(unsigned long obj_size, unsigned long nobjs)
{
	if (obj_size == 0 || nobjs == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero objs (obj_size = 0, or nobjs = 0)");

	void *ptr = mem_alloc(obj_size * nobjs);

	/* TODO: Implement a vectorized set to zero */
	memset(ptr, 0, obj_size * nobjs);
	
	return ptr;
}
