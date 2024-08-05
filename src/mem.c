#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <except.h>
#include <except/assert.h>
#include <stddef.h>
#include <stdio.h>

#include "utils.h"
#include "heap.h"
#include "page.h"

Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

int mem_find_chks_page(const uint8_t *ptr)
{
	for (size_t i = 0; i < heap_pages.size; i++) {
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[i]);
		if (page.ptr < ptr && ptr < page.end)
			return i;
	}

	return -1;
}

void *mem_alloc(unsigned long nbytes)
{
	if (nbytes == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");
	
	nbytes = aling_to_mul_8(nbytes + sizeof(uint64_t));
	nbytes = MAX((uint32_t) CHK_MIN_CHUNK_SIZE, nbytes);
	
	Chk_T chk = {
		.size = nbytes
	};

	if (heap_free_chunks.size > 0
	    && CHKPTR_SIZE(Heap_top(&heap_free_chunks)) >= nbytes) {
		uint8_t *frag_chkptr = Heap_pop(&heap_free_chunks, &Chk_capacity_cmp);
		Chk_T frag_chk = CHKPTR_FETCH_CHK_T(frag_chkptr);
		frag_chk.capacity -= chk.size;
		frag_chk.size -= chk.size;
		if (frag_chk.size >= CHK_MIN_CHUNK_SIZE) {
			chk.ptr = (uint8_t *) frag_chk.raddr + frag_chk.capacity;
			chk.capacity = chk.size - sizeof(uint64_t);
			
			/* Update the metadata */
			*((uint64_t *) chk.ptr) = chk.capacity;
			*((uint64_t *) frag_chk.ptr) = frag_chk.capacity;
			
			assert(CHKPTR_CAPACITY(chk.ptr) == (uint32_t) chk.capacity);
			assert(CHKPTR_CAPACITY(frag_chk.ptr) == (uint32_t) \
			       frag_chk.capacity);
			
			Heap_push(&heap_free_chunks, frag_chk.ptr, &Chk_capacity_cmp);
			return (void *) chk.raddr;
		}
		
		return (void *) frag_chk.raddr;
	}

	
	Page_T page;
	if (heap_pages.size > 0
	    && PAGEPTR_CAPACITY(Heap_top(&heap_pages)) > nbytes) {
		Page_T root_page = PAGEPTR_FETCH_PAGE_T(Heap_top(&heap_pages));
		page = root_page;
	} else {
		Page_alloc(&page, nbytes);
		Heap_push(&heap_pages, page.ptr, &Page_capacity_cmp);
	}

	Page_chk_alloc(&page, &chk);
	
	assert(chk.capacity > 0 && chk.capacity < chk.size);
	assert(chk.ptr > page.ptr);
	assert(chk.raddr < page.end);
	assert(chk.raddr < page.available);
	assert(heap_pages.size > 0);
			
	return (void *) chk.raddr;
}


void mem_free(void *addr)
{
	uint8_t *chkptr = (uint8_t *) addr - sizeof(uint64_t);
	Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);

	int page_index = mem_find_chks_page(chkptr);
	if (page_index == -1)
		RAISE(ExceptInvalidAddr, "Can't free an invalid addr");

	Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[page_index]);
	if (chk.capacity == 0 || chk.size >= page.size)
		RAISE(ExceptCorruptedAddr, "The reserved addr has an invalid capacity");

	if (page.available <= chk.ptr
	    || Heap_find(&heap_free_chunks, chk.ptr, &Chk_capacity_cmp) != -1)
		RAISE(ExceptInvalidAddr, "Address already freed");
	
	Page_chk_free(&page, &chk);

	if (page.available - 2 * sizeof(uint64_t) == page.ptr) {
		Heap_rem(&heap_pages, page_index, &Page_capacity_cmp);
		Page_free(&page);
	}
}

void *mem_ralloc(void *addr, unsigned long nbytes)
{
	if (nbytes == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");
	
	uint8_t *chkptr = (uint8_t *) addr - sizeof(uint64_t);
	Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);

	int page_index = mem_find_chks_page(chkptr);
	if (page_index == -1)
		RAISE(ExceptInvalidAddr, "Can't free an invalid addr");
	
	uint32_t n = MIN(nbytes, (uint64_t) chk.capacity);
	void *new_addr = mem_alloc(nbytes);

	/* TODO: Implement a vectorized memcpy */
	memcpy(new_addr, addr, n);

	mem_free(addr);
	
	return new_addr;
}

void *mem_calloc(unsigned long obj_size, unsigned long nobjs)
{
	if (obj_size == 0 || nobjs == 0)
		RAISE(ExceptInvalidNBytes, "Can't alloc zero objs (obj_size = 0, or nobjs = 0)");
	
	return mem_alloc(obj_size * nobjs);
}

int mem_dbg_is_freeded(void *addr)
{
	uint8_t *ptr = (uint8_t *) addr - sizeof(uint64_t);
	int page_ind = mem_find_chks_page(ptr);
	if (page_ind == -1)
		return 1;
	Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[page_ind]);
	Chk_T chk = CHKPTR_FETCH_CHK_T(ptr);
	
	if (chk.capacity == 0 || chk.size >= page.size)
		RAISE(ExceptCorruptedAddr, "The reserved addr has an invalid capacity");

	if (page.available <= page.ptr)
		return 1;

	int chk_ind = Heap_find(&heap_free_chunks, chk.ptr, &Chk_capacity_cmp);
	if (chk_ind != -1)
		return 1;

	return 0;
}


/* TODO: Set excetions to all the mem_dbg functions */

unsigned int mem_dbg_num_pages(void)
{
	return heap_pages.size;
}

void mem_dbg_dump_pages_buff(void **buff, unsigned int n)
{
	assert(buff != NULL);
	assert(n > 0);
	n = MIN(heap_pages.size, n);
	
	for (uint32_t i = 0; i < n; i++)
		buff[i] = heap_pages.buff[i];
}

void mem_dbg_dump_pages_info(void)
{
	uint32_t npages = mem_dbg_num_pages();
	void *buff[npages];

	mem_dbg_dump_pages_buff(buff, npages);


	for (uint32_t i = 0; i < npages; i++) {
		Page_T page = PAGEPTR_FETCH_PAGE_T(buff[i]);
		LOG_DBG_INF("<Page: %i, ptr: %p, available: %p, end: %p, size: %lu, "
			    "available capacity: %lu>",
			    (i + 1), page.ptr, page.available, page.end,
			    page.size, page.capacity);
		
	}
}

unsigned int mem_dbg_num_chks_in_page(const void *page_ptr)
{
	assert(page_ptr != NULL, "Can't be null");
	
	Page_T page = PAGEPTR_FETCH_PAGE_T(page_ptr);

	uint32_t nchunks = 0;
	
	if (page.ptr != NULL) {
		uint8_t *ptr = page.ptr + 2 * sizeof(uint64_t);
		while (ptr < page.available) {
			ptr += CHKPTR_SIZE(ptr);
			nchunks++;
		}
	}

	return nchunks;
}

void mem_dbg_dump_chks_from_page(const void *page_ptr, void **buff, unsigned int n)
{
	assert(buff != NULL && page_ptr != NULL, "Can't be null");
	assert(n > 0);

	Page_T page = PAGEPTR_FETCH_PAGE_T(page_ptr);
	
	uint32_t nchks = mem_dbg_num_chks_in_page(page.ptr);
	n = MIN(n, nchks);

	uint8_t *ptr = page.ptr + 2 * sizeof(uint64_t);
	for (uint32_t i = 0; i < n; i++) {
		buff[i] = ptr;
		ptr += CHKPTR_SIZE(ptr);
	}
	
}


void mem_dbg_dump_chks_info_from_page(const void *page_ptr)
{
	assert(page_ptr != NULL, "Can't be null");
	
	uint32_t nchks = mem_dbg_num_chks_in_page(page_ptr);
	uint32_t nfreed = 0, nonfreed = 0, usedmem = 0, nonusedmem = 0;
	void *buff[nchks];
	mem_dbg_dump_chks_from_page(page_ptr, buff, nchks);
	
	for (uint32_t i = 0; i < nchks; i++) {
		Chk_T chk = CHKPTR_FETCH_CHK_T(buff[i]);
		int freed = Heap_find(&heap_free_chunks, (const void *) buff[i],
				      &Chk_capacity_cmp) != -1;
		
		LOG_DBG_INF("<Chunk: %i, ptr: %p, reserved address: %p, end: %p, size: %i, "
			    "capacity: %i, freed: %s>",
			    (i + 1), chk.ptr, chk.raddr, chk.end, chk.size, chk.capacity,
			    (freed) ? "true" : "false");
		if (freed) {
			nfreed++;
			nonusedmem += chk.size;
		} else {
			nonfreed++;
			usedmem += chk.size;
		}
	}
	
	nonusedmem += PAGEPTR_CAPACITY(page_ptr);

	LOG_DBG_INF("Stats from Page. freed chunks: %u (%.1f%%), non freed chunks: %u (%.1f%%), total chunks: %u, "
		    "not used mem: %u (%.1f%%), used mem: %u (%.1f%%)",
		    nfreed, (float) nfreed * 100 / (nchks > 0 ? nchks : 1),
		    nonfreed, (float) nonfreed * 100 / (nchks > 0 ? nchks : 1), nchks,
		    nonusedmem, (float) nonusedmem * 100 / PAGEPTR_SIZE(page_ptr),
		    usedmem, (float) usedmem * 100 / PAGEPTR_SIZE(page_ptr));
}


void mem_dbg_dump_info(void)
{
	uint32_t npages = mem_dbg_num_pages();
	
	for (uint32_t i = 0; i < npages; i++) {
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[i]);
		LOG_DBG_INF("<Page: %i, ptr: %p, available: %p, end: %p, size: %lu, "
			    "available capacity: %lu>",
			    (i + 1), page.ptr, page.available, page.end,
			    page.size, page.capacity);
		mem_dbg_dump_chks_info_from_page(heap_pages.buff[i]);
	}
}

unsigned int mem_dbg_num_chks(void)
{
	uint32_t nchks = 0;
	uint32_t npages = mem_dbg_num_pages();
	
	for (uint32_t i = 0; i < npages; i++)
		nchks += mem_dbg_num_chks_in_page(heap_pages.buff[i]);
	return nchks;
}

void mem_dbg_dump_chks_buff(void **buff, unsigned int n)
{
	uint32_t nchks = mem_dbg_num_chks(), cchks = 0;
	long nbuff = MIN(n, nchks);
	uint32_t npages = mem_dbg_num_pages();
	
	for (uint32_t i = 0; i < npages; i++) {
		uint32_t nchks_cpage = mem_dbg_num_chks_in_page(heap_pages.buff[i]);;
		mem_dbg_dump_chks_from_page(heap_pages.buff[i], buff + cchks, nbuff);
		nbuff -= nchks_cpage;
		if (nbuff <= 0)
			break;
		cchks += nchks_cpage;
	}
}

void mem_dbg_dump_chks_info(void)
{
	uint32_t nchks = mem_dbg_num_chks();
	uint32_t npages = mem_dbg_num_pages();
	void *buff[nchks];
	uint32_t nfreed = 0, nonfreed = 0, usedmem = 0, nonusedmem = 0;
	uint32_t chk_max_size = 0, chk_min_size = UINT32_MAX;
	double chk_avg_size = 0.0;
	
	mem_dbg_dump_chks_buff(buff, nchks);
	
	for (uint32_t i = 0; i < nchks; i++) {
		Chk_T chk = CHKPTR_FETCH_CHK_T(buff[i]);
		int freed = Heap_find(&heap_free_chunks, (const void *) buff[i],
				      &Chk_capacity_cmp) != -1;
		chk_max_size = MAX(chk_max_size, (uint32_t) chk.size);
		chk_min_size = MIN(chk_min_size, (uint32_t) chk.size);
		chk_avg_size += (double) chk.size;
		
		LOG_DBG_INF("<Chunk: %i, ptr: %p, reserved address: %p, end: %p, size: %i, "
			    "capacity: %i, freed: %s>",
			    (i + 1), chk.ptr, chk.raddr, chk.end, chk.size, chk.capacity,
			    (freed) ? "true" : "false");
		if (freed) {
			nfreed++;
			nonusedmem += chk.size;
		} else {
			nonfreed++;
			usedmem += chk.size;
		}
	}

	uint32_t total_mem = 0;
	for (uint32_t i = 0; i < npages; i++) {
		nonusedmem += PAGEPTR_CAPACITY(heap_pages.buff[i]);
		total_mem += PAGEPTR_SIZE(heap_pages.buff[i]);
	}

	chk_avg_size /= nchks;

	LOG_DBG_INF("Stats from all pages (num pages: %u). freed chunks: %u (%.1f%%), non freed chunks: %u (%.1f%%), "
		    "total chunks: %u,\n max chk size: %u, min chk size: %u, avg chk size: %0.1lf, not used mem: %u (%.1f%%), "
		    "used mem: %u (%.1f%%), total mem: %u bytes",
		    npages, nfreed, (float) nfreed * 100 / (nchks > 0 ? nchks : 1),
		    nonfreed, (float) nonfreed * 100 / (nchks > 0 ? nchks : 1), nchks, chk_max_size, chk_min_size, chk_avg_size,
		    nonusedmem, (float) nonusedmem * 100 / (total_mem > 0 ? total_mem : 1),
		    usedmem, (float) usedmem * 100 / (total_mem > 0 ? total_mem : 1), total_mem);
}


