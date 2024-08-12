#include <stdio.h>
#include <inttypes.h>
#include <except/assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define FULL_MEM_DBG
#include "include/mem.h"
#include "src/heap.h"
#include "src/utils.h"
#include "src/chk.h"
#include "src/page.h"

void foo(void *addr)
{
	assert(!mem_dbg_is_freeded(addr));
}

#ifndef NDEBUG
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
#endif

void reset_the_allocator(void)
{
	for (int i = 0; i < heap_pages.size; i++) {
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[i]);
		Page_free(&page);
	}

	heap_pages.size = 0;
	heap_free_chunks.size = 0;
}


void random_allocs(int max_mem_to_alloc, int nallocations)
{
	int nfrees = 0, buff_size = 0;
	void *allocs[nallocations];
	bzero(allocs, sizeof(allocs));
	clock_t start, end;
	
	
	start = clock();
	for (int i = 0; i < nallocations; i++) {
		/* puts("---------------------------------------------------"); */
		/* mem_dbg_dump_chks_info(); */
		
		/* LOG_DBG_INF("allocations: %i, frees: %i, buff_size: %i", i + 1, nfrees, buff_size); */
		allocs[buff_size++] = mem_alloc((rand() % max_mem_to_alloc) + 1);
		
		/* LOG_DBG_INF("allocating: %p", allocs[i]); */

		/* puts(""); */
		/* mem_dbg_dump_chks_info(); */
		/* puts(""); */
		
		if (rand() % 2 == 0) {
			uint32_t q = rand() % (buff_size);
			/* while (allocs[q] == NULL) */
			/* 	q = rand() % (i + 1); */
			assert(allocs[q] != NULL);
			
			/* LOG_DBG_INF("freeing: %p", allocs[q]); */
			mem_free(allocs[q]);
			
			allocs[q] = allocs[--buff_size];
			allocs[buff_size] = NULL;
			nfrees++;
		}
		/* mem_dbg_dump_chks_info(); */
	}

	end = clock();
	double lapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	
	/* Heap_verify_integrity(&heap_free_chunks, &Chk_capacity_cmp); */
	/* Heap_verify_integrity(&heap_pages, &Page_capacity_cmp); */

	extern void mem_dbg_dump_stats_info(void);
	mem_dbg_dump_stats_info();
	LOG_DBG_INF("Total number of allocations: %u, Total number of frees: %u (%0.1f%%), time: %0.5lf",
		    nallocations, nfrees, (float) nfrees * 100 / nallocations, lapsed);
}



int main(void)
{
	/* simple_allocs(); */
	
	/* for (int i = 1; i <= KILOBYTE; i++) { */
	/* 	random_allocs(i * KILOBYTE, 1000); */
	/* 	reset_the_allocator(); */
	/* } */
	
	/* Un Released version */
	/* 256, 1000 -> 0.00382 */
	/* 1KB, 1000 -> 0.00954 */
	/* 4 * 1KB, 1000 -> 0.01531 */
	/* 10 * 1KB, 1000 -> 0.02232 */
	/* 50 * 1KB, 1000 -> 0.02337 */
	/* 100 * 1KB, 1000 -> 0.02059 */
	/* 1MB, 1000 -> 0.04536  */
	/* 4 * 1MB, 1000 -> 0.20541  */
	/* 10 * 1MB, 1000 -> 0.28314  */
	/* 100 * 1MB, 1000 -> 0.36433  */
	/* 1GB, 1000 -> 0.35325  */
	

	/* Released version */

	/* 100 * 1KB, 1000 -> 0.02059 */
	/* 100 * 1KB, 1000 -> 0.01533  */
	/* 1MB, 1000 ->  0.01012 */
	/* 4 * 1MB, 1000 -> 0.16559  */
	/* 10 * 1MB, 1000 -> 0.24446  */
	/* 100 * 1MB, 1000 -> 0.28581  */
	/* 1GB, 1000 -> 0.33756  */
	
	random_allocs(4 * KILOBYTE, 1000);
	
	
	return 0;
}




