#include <stdio.h>
#include <inttypes.h>
#include <except/assert.h>
#include <stdlib.h>
#include <time.h>

#define MEM_DBG
#include "include/mem.h"
#include "src/heap.h"
#include "src/utils.h"


void foo(void *addr)
{
	assert(!mem_dbg_is_freeded(addr));
}


void simple_allocs(void)
{
	uint8_t *ptr, *ptr2, *ptr3;

	/* Simple allocation */
	ptr = mem_alloc(100);
	ptr2 = mem_alloc(40);
	ptr3 = mem_alloc(200);

	
	int npages = mem_dbg_num_pages();
	void *buff[npages];
	mem_dbg_dump_pages_buff(buff, npages);
	
	assert(npages == 1);

	int nchks = mem_dbg_num_chks_in_page(buff[0]);
	assert(nchks == 3);
	
	mem_dbg_dump_chks_info();	
	mem_free(ptr);
	puts("---------------------------");

	mem_dbg_dump_chks_info();
	mem_free(ptr2);
	puts("---------------------------");
	
	mem_dbg_dump_chks_info();	
	mem_free(ptr3);
	puts("---------------------------");

	mem_dbg_dump_chks_info();
}


int main(void)
{
	/* simple_allocs(); */

	int nallocations = 1000, nfrees = 0;
	void *allocs[nallocations];
	int max_mem_to_alloc = KILOBYTE;
	
	for (int i = 0; i < nallocations; i++) {
		/* puts("---------------------------------------------------"); */
		/* mem_dbg_dump_chks_info(); */
		
		allocs[i] = mem_alloc((rand() % max_mem_to_alloc) + 1);
		/* LOG_DBG_INF("allocating: %p", allocs[i]); */
		
		/* puts(""); */
		/* mem_dbg_dump_chks_info(); */
		/* puts(""); */
		
		if (rand() % 2 == 0) {
			uint32_t q = rand() % (i + 1);
			while (allocs[q] == NULL)
				q = rand() % (i + 1);
			/* LOG_DBG_INF("freeing: %p", allocs[q]); */
			mem_free(allocs[q]);
			allocs[q] = NULL;
			nfrees++;
		}

		
		/* mem_dbg_dump_chks_info(); */
	}

	mem_dbg_dump_chks_info();
	LOG_DBG_INF("Total number of allocations: %u, Total number of frees: %u (%0.1f%%)", nallocations, nfrees, (float) nfrees * 100 / nallocations);
	
	return 0;
}




