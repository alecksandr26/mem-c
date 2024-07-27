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

/* TODO: Change all this code to work for Page_T */

#define MAX_CAP_CHUNKS_INFO 1024

Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

void *mem_alloc(unsigned long nbytes)
{
	if (nbytes == 0)
		RAISE(ExceptInvalidNBytes, "nbytes = 0");

	if (start_brk == NULL)
		if ((start_brk = sbrk(0)) == (void *) -1)
			RAISE(ExceptFatalHeapError, "sbrk: %s", strerror(errno));

	uint8_t *ptr;
	assert(aling_to_mul_8(nbytes + sizeof(uint64_t)) >= nbytes + sizeof(uint64_t));
	nbytes = aling_to_mul_8(nbytes + sizeof(uint64_t));
	nbytes = MAX(nbytes, MEM_ALLOC_MIN_CHUNK_SIZE);
	
	if (heap_free_chunks.size && CHUNK_SIZE(Heap_top(&heap_free_chunks)) > nbytes) {
		uint8_t *frag_chunk = Heap_pop(&heap_free_chunks);
		uint32_t frag_chunk_size = CHUNK_SIZE(frag_chunk) - nbytes;
		if (frag_chunk_size >= MEM_ALLOC_MIN_CHUNK_SIZE) {
			ptr = frag_chunk + frag_chunk_size;
			*((uint64_t *) ptr) = nbytes - sizeof(uint64_t);
			ptr += sizeof(uint64_t);
			assert(CHUNK_SIZE(ptr) == nbytes - sizeof(uint64_t));
			CHUNK_SIZE(frag_chunk) = frag_chunk_size;
			assert(CHUNK_SIZE(frag_chunk) == frag_chunk_size);
			
			if (heap_free_chunks.size == HEAP_CAPACITY)
				RAISE(ExceptOverFreededChunks, "Can't free more");
			
			Heap_push(&heap_free_chunks, frag_chunk);
			return ptr;
		}
		ptr = frag_chunk;
	} else {
		if ((ptr = sbrk(nbytes)) == (void *) - 1)
			RAISE(ExceptFatalHeapError, "sbrk: %s", strerror(errno));
	
		end_brk = ptr + nbytes;
		*((uint64_t *) ptr) = nbytes - sizeof(uint64_t);
		ptr += sizeof(uint64_t);
		assert(CHUNK_SIZE(ptr) == nbytes - sizeof(uint64_t));
	}
	
	return ptr;
}

/* TODO: Implement the realloc and calloc */
void *mem_ralloc(void *addr, unsigned long nbytes)
{
	assert(0, "Unimplemented");
}

void *mem_calloc(unsigned long obj_size, unsigned long nobjs)
{
	assert(0, "Unimplemented");
}

void mem_free(void *addr)
{
	uint8_t *ptr = addr;
	if (ptr < start_brk
	    || ptr > end_brk)
		RAISE(ExceptInvalidAddr, "addr out of range in the heap");
	if (CHUNK_SIZE(ptr) > (uint64_t) (end_brk - ptr))
		RAISE(ExceptCorruptedAddr, "size of addr is out of range");


	if (CHUNK_SIZE(ptr) == (uint64_t) (end_brk - ptr)) {
		end_brk = ptr - sizeof(uint64_t);
		brk(end_brk);
		return;
	}

	if (heap_free_chunks.size == HEAP_CAPACITY)
		RAISE(ExceptOverFreededChunks, "Can't free more");

	Heap_push(&heap_free_chunks, addr);
}

/* TODO: Change this utilities to work with pages */

int mem_dbg_is_freeded(void *addr)
{
	uint8_t *ptr = addr;
	if (ptr < start_brk
	    || ptr > (uint8_t *) &ptr)
		RAISE(ExceptInvalidAddr, "addr out of range in the heap");

	return ptr >= end_brk || Heap_find(&heap_free_chunks, addr) != -1;
}

unsigned int mem_dbg_num_chunks(void)
{
	uint32_t nchunks = 0;
	
	if (start_brk != NULL) {
		uint8_t *ptr = start_brk;
		while (1) {
			ptr += sizeof(uint64_t);
			if (ptr >= end_brk)
				break;
			ptr += CHUNK_SIZE(ptr);
			nchunks++;
		}
	}

	return nchunks;
}

void mem_dbg_dump_chunks_buff(void **buff, int n)
{
	uint32_t nchunks = mem_dbg_num_chunks();
	n = MIN(n, (int) nchunks);

	uint8_t *ptr = start_brk;
	for (int i = 0; i < n; i++) {
		ptr += sizeof(uint64_t);
		buff[i] = ptr;
		ptr += CHUNK_SIZE(ptr);
	}
}

void mem_dbg_dump_chunks_info(void)
{
	uint32_t nchunks = mem_dbg_num_chunks();
	uint32_t nfreededchunks = 0, nnonfreededchunks = 0;
	char chunks_info[MAX_CAP_CHUNKS_INFO][256];
	uint8_t *buff[nchunks];
	
	mem_dbg_dump_chunks_buff((void **) buff, nchunks);

	uint8_t *ptr = start_brk;
	for (uint32_t i = 0; i < nchunks && i < MAX_CAP_CHUNKS_INFO; i++) {
		ptr += sizeof(uint64_t);
		if (!mem_dbg_is_freeded(ptr))
			nnonfreededchunks++;
		else
			nfreededchunks++;
		sprintf(chunks_info[i],
			"Num Chunk: %i, start: %p, end: %p, size: %lu, free: %s",
			i + 1, ptr - sizeof(uint64_t), ptr + CHUNK_SIZE(ptr),
			CHUNK_SIZE(ptr), ((Heap_find(&heap_free_chunks, ptr) == -1)
					  ? "false"
					  : "true"));
		ptr += CHUNK_SIZE(ptr);
	}

	LOG_DBG_INF("Total Num of Chunks: %i, freeded: %i (%.1f %%), nonfreeded: %i (%.1f %%), "
		    "start_brk: %p, end_brk: %p",
		    nchunks, nfreededchunks, (float) (nfreededchunks * 100 / ((nchunks > 0 ? nchunks : 1))),
		    nnonfreededchunks, (float) (nnonfreededchunks * 100 / ((nchunks > 0 ? nchunks : 1))),
						      start_brk, end_brk);
	for (size_t i = 0; i < nchunks; i++)
		LOG_DBG_INF("%s", chunks_info[i]);
}


