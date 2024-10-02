#include <stdio.h>
#include <inttypes.h>
#include <except/assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Simple demo of the project */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "include/mem.h"

#define NDEBUG

#include "src/heap.h"
#include "src/utils.h"
#include "src/chk.h"
#include "src/page.h"


/* A more debuggable code */
void foo(void *addr)
{
	assert(!mem_dbg_is_freeded(addr));
	
}

void reset_the_allocator(void)
{
	for (size_t i = 0; i < heap_pages.size; i++) {
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[i]);
		Page_free(&page);
	}

	heap_pages.size = 0;
	heap_free_chunks.size = 0;
}

double random_allocs(MemStats_T *stats, int alloc_size, int nallocations)
{
	int nfrees = 0, buff_size = 0;
	void *allocs[nallocations];
	bzero(allocs, sizeof(allocs));
	clock_t start, end;
	
	
	start = clock();
	for (int i = 0; i < nallocations; i++) {		
		allocs[buff_size++] = mem_alloc((rand() % alloc_size) + 1);
				
		if (rand() % 2 == 0) {
			uint32_t q = rand() % (buff_size);
			
			assert(allocs[q] != NULL);
			
			
			mem_free(allocs[q]);
			
			allocs[q] = allocs[--buff_size];
			allocs[buff_size] = NULL;
			nfrees++;
		}
	}

	end = clock();
	double lapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	
	mem_dbg_verify_ds_integrity();
	mem_dbg_fetch_mem_stats(stats, 0, 0);
	
	reset_the_allocator();

	return lapsed;
}




int main(void)
{
	MemStats_T stats;

	/* Number of test cases */
	int N = 275;
	
	for (int i = 5; i < N; i++) {
		int nallocations = (rand() % (10 * i)) + 10 * (i - 4);
		int alloc_size = (rand() % (128 * i)) + 128 * (i - 4);
		
		double elapsed = random_allocs(&stats, alloc_size, nallocations);
		LOG_DBG_INF("n allocs: %i, alloc size: %i, non used memory by user: %u, "
			    "non used memory by user percetnage: %lf, elapsed time: %lf, "
			    "freeded chunks percentage: %lf, non freeded chunks percetnage: %lf",
			    nallocations, alloc_size, stats.nonusedmem_byu, stats.nonusedmem_p_byu,
			    elapsed, stats.nfreedchks_p, stats.nnonfreedchks_p);
	}
	
	
	return 0;
}




