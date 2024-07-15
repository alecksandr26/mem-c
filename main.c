#include <stdio.h>
#include <inttypes.h>
#include <except/assert.h>
#include <stdlib.h>
#include <time.h>

#include "include/mem.h"
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

	mem_dbg_dump_chunks_info();
	puts("");

	mem_free(addr2);

	mem_dbg_dump_chunks_info();
	puts("");
	
	void *addr4 = mem_alloc(4);

	assert(heap_free_chunks.size == 1);
	
	mem_dbg_dump_chunks_info();
	puts("");
}


void foo(void *addr)
{
	assert(!mem_dbg_is_freeded(addr));
}



uint8_t *find_nonfree_chunk(void)
{
	unsigned int n = mem_dbg_num_chunks();
	void *buff[n];

	mem_dbg_dump_chunks_buff(buff, n);
	
	while (1) {
		unsigned int i = rand() % n;
		if (!mem_dbg_is_freeded(buff[i]))
			return buff[i];
	}

	assert(0);
}

int main(void)
{
	srand(time(0));
	
	MEM_ALLOC_MIN_CHUNK_SIZE = 8;
	
#define N 100

	for (int i = 0; i < N; i++) {
		printf("----------------- %i -----------------\n", i);
		int n = (rand() % 123) + 1;
		void *ptr = mem_alloc(n);
		mem_dbg_dump_chunks_info();
		puts("");
		
		if (i % 2 == 0 && mem_dbg_num_chunks() > 0) {
			ptr = find_nonfree_chunk();
			mem_free(ptr);
			puts("Free memory");
		}

		mem_dbg_dump_chunks_info();
		puts("");
	}


	/* test1(); */
	
	
	return 0;
}




