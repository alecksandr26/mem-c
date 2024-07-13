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

#define MAX_CAP_CHUNKS_INFO 1024

Except_T ExceptFatalHeapError = INIT_EXCEPT_T("Fatal error in heap's data segment ");
Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

uint8_t *start_brk = NULL;
uint8_t *end_brk = NULL;

Heap_T heap_free_chunks = {
	.size = 0
};

unsigned int MEM_ALLOC_MIN_CHUNK_SIZE = 40;

static inline uint64_t aling_to_mul_8(uint64_t n)
{
	return n + (8 - (n % 8));
}

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

	/* TODO: Add exception of the limit of free chunks */
	Heap_push(&heap_free_chunks, addr);
}

void mem_dbg_dump_chunks_info(void)
{
	size_t nchuks = 0;
	char chunks_info[MAX_CAP_CHUNKS_INFO][256];
	
	if (start_brk != NULL) {
		uint8_t *ptr = start_brk;
		while (ptr < end_brk && nchuks < MAX_CAP_CHUNKS_INFO) {
			ptr += sizeof(uint64_t);
			sprintf(chunks_info[nchuks],
				"Num Chunk: %lu, start: %p, end: %p, size: %lu, free: %s",
				nchuks + 1, ptr - sizeof(uint64_t), ptr + CHUNK_SIZE(ptr),
				CHUNK_SIZE(ptr), ((Heap_find(&heap_free_chunks, ptr) == -1)
						  ? "false"
						  : "true"));
			ptr += CHUNK_SIZE(ptr);
			nchuks++;
		}
	}

	printf("Total Num of Chunks: %lu, start_brk: %p, end_brk: %p \n",
	       nchuks, start_brk, end_brk);
	for (size_t i = 0; i < nchuks; i++)
		printf("%s\n", chunks_info[i]);
}
