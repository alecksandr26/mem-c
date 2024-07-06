#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <except.h>
#include <except/assert.h>
#include <stddef.h>
#include <stdio.h>

#include "utils.h"

#define MAX_CAP_CHUNKS_INFO 1024


Except_T ExceptFatalHeapError = INIT_EXCEPT_T("Fatal error in heap's data segment ");
Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

uint8_t *start_brk = NULL;
uint8_t *end_brk = NULL;


static inline uint64_t aling_to_mul_8(uint64_t n)
{
	return n + (8 - (n % 8));
}

/* TODO: Implement the heap */

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
	if ((ptr = sbrk(nbytes)) == (void *) - 1)
		RAISE(ExceptFatalHeapError, "sbrk: %s", strerror(errno));
	
	end_brk = ptr + nbytes;
	*((uint64_t *) ptr) = nbytes - sizeof(uint64_t);
	ptr += sizeof(uint64_t);
	
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
	if (CHUK_SIZE(ptr) > (uint64_t) (end_brk - ptr))
		RAISE(ExceptCorruptedAddr, "size of addr is out of range");

	if (CHUK_SIZE(ptr) == (uint64_t) (end_brk - ptr)) {
		end_brk = ptr - sizeof(uint64_t);
		brk(end_brk);
		return;
	}
}

void mem_dump_heap_stdout(void)
{
	size_t nchuks = 0;
	char chunks_info[MAX_CAP_CHUNKS_INFO][256];
	
	if (start_brk != NULL) {
		uint8_t *ptr = start_brk;
		while (ptr < end_brk) {
			ptr += sizeof(uint64_t);
			sprintf(chunks_info[nchuks],
				"Num Chunk: %lu, start: %p, end: %p, size: %lu",
				nchuks + 1, ptr - sizeof(uint64_t), ptr + CHUK_SIZE(ptr),
				CHUK_SIZE(ptr));
			ptr += CHUK_SIZE(ptr);
			nchuks++;
		}
	}

	printf("Total Num of Chunks: %lu, start_brk: %p, end_brk: %p \n",
	       nchuks, start_brk, end_brk);
	for (size_t i = 0; i < nchuks; i++)
		printf("%s\n", chunks_info[i]);
}
