#include <stdio.h>
#include <inttypes.h>
#include <except/assert.h>
#include <stdlib.h>
#include <time.h>

#include "include/mem_alloc.h"
#include "src/heap.h"
#include "src/utils.h"

extern uint8_t *start_brk;
extern uint8_t *end_brk;
extern unsigned int MEM_ALLOC_MIN_CHUNK_SIZE;
extern Heap_T heap_free_chunks;

void test1(void)
{
	MEM_ALLOC_MIN_CHUNK_SIZE = 8;
	
	void *addr = mem_alloc(10);
	void *addr2 = mem_alloc(20);
	void *addr3 = mem_alloc(30);

	mem_dump_chunks_info();
	puts("");

	mem_free(addr2);

	mem_dump_chunks_info();
	puts("");
	
	void *addr4 = mem_alloc(4);

	assert(heap_free_chunks.size == 1);
	
	mem_dump_chunks_info();
	puts("");
}


uint8_t *find_nonfree_chunk(void)
{
	if (start_brk != NULL) {
		uint8_t *ptr = start_brk;
		while (ptr < end_brk) {
			ptr += sizeof(uint64_t);

			if (Heap_find(&heap_free_chunks, ptr) == -1)
				return ptr;
			ptr += CHUNK_SIZE(ptr);
		}
	}

	assert(0);
}


int main(void)
{
	srand(time(0));
	
#define N 100

	for (int i = 0; i < N; i++) {
		printf("----------------- %i -----------------\n", i);
		int n = (rand() % 123) + 1;
		void *ptr = mem_alloc(n);
		mem_dump_chunks_info();
		puts("");
		
		if (rand() % 2 == 0 && (int) (end_brk - start_brk) > 0) {
			ptr = find_nonfree_chunk();
			mem_free(ptr);
			puts("Free memory");
		}

		mem_dump_chunks_info();
		puts("");
		
	}
	
	return 0;
}




