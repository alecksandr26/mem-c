#include <stdint.h>
#include <except/assert.h>
#include <string.h>

#include "utils.h"
#ifdef NDEBUG
#undef NDEBUG
#endif


#include "../include/mem.h"

#define NDEBUG

#include "page.h"
#include "heap.h"
#include "chk.h"

Except_T ExceptCorruptedHeapDS = INIT_EXCEPT_T("Corrupted core data structure");

/* Simple print statements debug functions */

static void mem_dbg_fetch_memstats_from_page(const int8_t *page_ptr, MemStats_T *stats, int verbose)
{
	assert(page_ptr != NULL || stats != NULL, "Can't be null");
	Page_T page = PAGEPTR_FETCH_PAGE_T(page_ptr);

	stats->nchks = 0;
	stats->nfreedchks = 0;
	stats->nnonfreedchks = 0;
	stats->avgchksize = 0.0;
	
	stats->maxchksize = INT32_MIN;
	stats->minchksize = INT32_MAX;

	stats->totalmem = page.size;
	stats->nonusedmem_byu = stats->nonusedmem = page.capacity;
	stats->usedmem_byu = stats->usedmem = page.size - page.capacity;
	stats->usedmem_p = (double) stats->usedmem * 100.0 / stats->totalmem;
	stats->nonusedmem_p = (double) stats->nonusedmem * 100.0 / stats->totalmem;
	

	if (verbose == 3)
		LOG_DBG_INF("----------------------------------------------------------------"
			    "----------------------------------------------------------------");
	
	
	uint8_t *ptr = page.ptr + 2 * sizeof(uint64_t);
	while (ptr < page.available) {
		stats->nchks++;
		Chk_T chk = CHKPTR_FETCH_CHK_T(ptr);
		int freed = Chk_verify_checksum(&chk) == 1;
			
		if (freed) {
			stats->nfreedchks++;
			stats->usedmem_byu -= chk.size;
			stats->nonusedmem_byu += chk.size;
		} else {
			stats->nnonfreedchks++;
			stats->usedmem_byu -= sizeof(uint64_t);
			stats->nonusedmem_byu += sizeof(uint64_t);
		}


		if (stats->maxchksize < chk.size) stats->maxchksize = chk.size;
		if (stats->minchksize > chk.size) stats->minchksize = chk.size;
			
			
		stats->avgchksize += (double) chk.size;
		ptr += chk.size;


		if (verbose == 3)
			LOG_DBG_INF("<Chunk: %i, ptr: %p, reserved address: %p, end: %p, size: %i, "
				    "capacity: %i, freed: %s>",
				    stats->nchks, chk.ptr, chk.raddr, chk.end, chk.size, chk.capacity,
				    (freed) ? "true" : "false");
	}
		
	if (stats->nchks) {
		stats->avgchksize /= stats->nchks;
		stats->nfreedchks_p = (double) stats->nfreedchks * 100.0 / (double) stats->nchks;
		stats->nnonfreedchks_p = (double) stats->nnonfreedchks * 100.0 / (double) stats->nchks;
	}


	stats->nonusedmem_p_byu = (double) stats->nonusedmem_byu * 100.0 / (double) stats->totalmem;
	stats->usedmem_p_byu = (double) stats->usedmem_byu * 100.0 / (double) stats->totalmem;
	
	if (verbose == 3)
		LOG_DBG_INF("----------------------------------------------------------------"
			    "----------------------------------------------------------------");
}

void mem_dbg_fetch_mem_stats(MemStats_T *stats, int verbose, int log_fd)
{
	if (stats == NULL)
		return;
	
	if (log_fd < 1)
		log_fd = 1;
	
	if (verbose > 3 || verbose < 0)
		verbose = 1;

	/* Change the log output */
	default_log_fd = log_fd;
	stats->nchks = 0;
	stats->nfreedchks = 0;
	stats->nnonfreedchks = 0;
	stats->avgchksize = 0.0;

	stats->maxchksize = INT32_MIN;
	stats->minchksize = INT32_MAX;
	
	stats->npages = heap_pages.size;
	stats->minpagesize = INT32_MAX;
	stats->maxpagesize = INT32_MIN;
	stats->minpagecp = INT32_MAX;
	stats->maxpagecp = INT32_MIN;
	stats->avgpagesize = 0.0;
	stats->avgpagecp = 0.0;

	stats->totalmem = 0;
	stats->usedmem = 0;
	stats->usedmem_byu = 0;
	stats->nonusedmem = 0;
	stats->nonusedmem_byu = 0;

	stats->minpagenchks = INT32_MAX;
	stats->maxpagenchks = INT32_MIN;
	stats->avgpagenchks = 0.0;
	
	for (int i = 0; i < stats->npages; i++) {
		MemStats_T stats_page;
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[i]);
		if (verbose >= 2) {
			LOG_DBG_INF("<Page: %i, ptr: %p, available: %p, end: %p, size: %lu,\n "
				    "available capacity: %lu>",
				    (i + 1), page.ptr, page.available, page.end,
				    page.size, page.capacity);
		}
		
		mem_dbg_fetch_memstats_from_page(heap_pages.buff[i], &stats_page, verbose);
		
		if (verbose >= 2) {
			LOG_DBG_INF("\n Summary stats from (num page: %i). total chunks: %u, freeded chunks: "
				    "%u (%.1f%%), "
				    "non freed chunks: %u (%.1f%%),\n max chunk size: %u, min chunk size: %u, "
				    "avg chunk size: %f,\n total mem: %u bytes, non used mem: %u bytes (%.1f%%),"
				    " used mem: %u bytes (%.1f%%),\n non used mem by user: %u bytes (%.1f%%), "
				    "used mem by user: %u bytes (%.1f%%).\n",
				    (i + 1), stats_page.nchks, stats_page.nfreedchks, stats_page.nfreedchks_p,
				    stats_page.nnonfreedchks, stats_page.nnonfreedchks_p,
				    stats_page.maxchksize, stats_page.minchksize, stats_page.avgchksize,
				    stats_page.totalmem, stats_page.nonusedmem, stats_page.nonusedmem_p,
				    stats_page.usedmem, stats_page.usedmem_p, stats_page.nonusedmem_byu,
				    stats_page.nonusedmem_p_byu, stats_page.usedmem_byu,
				    stats_page.usedmem_p_byu
				    );
		}

		/* Add the new calculated results */
		stats->nchks += stats_page.nchks;
		stats->nfreedchks += stats_page.nfreedchks;
		stats->nnonfreedchks += stats_page.nnonfreedchks;
		stats->avgchksize += stats_page.avgchksize;
		
		if (stats->maxchksize < stats_page.maxchksize) stats->maxchksize = stats_page.maxchksize;
		if (stats->minchksize > stats_page.minchksize) stats->minchksize = stats_page.minchksize;
		if (stats->maxpagesize < page.size) stats->maxpagesize = page.size;
		if (stats->minpagesize > page.size) stats->minpagesize = page.size;
		if (stats->maxpagecp < page.capacity) stats->maxpagecp = page.capacity;
		if (stats->minpagecp > page.capacity) stats->minpagecp = page.capacity;
		if (stats->maxpagenchks < stats_page.nchks) stats->maxpagenchks = stats_page.nchks;
		if (stats->minpagenchks > stats_page.nchks) stats->minpagenchks = stats_page.nchks;

		stats->totalmem += stats_page.totalmem;
		stats->usedmem += stats_page.usedmem;
		stats->usedmem_byu += stats_page.usedmem_byu - 2 * sizeof(uint64_t);
		stats->nonusedmem += stats_page.nonusedmem;
		stats->nonusedmem_byu += stats_page.nonusedmem_byu + 2 * sizeof(uint64_t);
		stats->avgpagesize += (double) page.size;
		stats->avgpagecp += (double) page.capacity;
		stats->avgpagenchks += (double) stats_page.nchks;
	}

	if (stats->npages) {
		stats->avgchksize /= stats->npages;
		stats->avgpagesize /= stats->npages;
		stats->avgpagecp /= stats->npages;
		stats->avgpagenchks /= stats->npages;
		stats->nfreedchks_p = (double) stats->nfreedchks * 100.0 / (double) stats->nchks;
		stats->nnonfreedchks_p = (double) stats->nnonfreedchks * 100.0 / (double) stats->nchks;
	}
	
	stats->usedmem_p = (double) stats->usedmem * 100.0 / stats->totalmem;
	stats->nonusedmem_p = (double) stats->nonusedmem * 100.0 / stats->totalmem;
	stats->nonusedmem_p_byu = (double) stats->nonusedmem_byu * 100.0 / (double) stats->totalmem;
	stats->usedmem_p_byu = (double) stats->usedmem_byu * 100.0 / (double) stats->totalmem;

	if (verbose >= 1) {
		LOG_DBG_INF("\n Summary stats total pages: %u, max page size: %u, min page size: %u, "
			    "avg page size: %.3f,\n max page capacity: %u, min page capacity: %u, "
			    "avg page capacity: %.3f,\n min num chunks in pages: %u, "
			    "max num chunks in pages: %u, avg num chunks in pages: %.3f,\n "
			    "total chunks: %u, freeded chunks: %u (%.1f%%), "
			    "non freed chunks: %u (%.1f%%),\n max chunk size: %u, min chunk size: %u, "
			    "avg chunk size: %.3f,\n total mem: %u bytes, non used mem: %u bytes (%.1f%%),"
			    " used mem: %u bytes (%.1f%%),\n non used mem by user: %u bytes (%.1f%%), "
			    "used mem by user: %u bytes (%.1f%%).\n",
			    stats->npages, stats->maxpagesize, stats->minpagesize, stats->avgpagesize,
			    stats->maxpagecp, stats->minpagecp, stats->avgpagecp, stats->minpagenchks,
			    stats->maxpagenchks, stats->avgpagenchks, 
			    stats->nchks, stats->nfreedchks, stats->nfreedchks_p,
			    stats->nnonfreedchks, stats->nnonfreedchks_p,
			    stats->maxchksize, stats->minchksize, stats->avgchksize,
			    stats->totalmem, stats->nonusedmem, stats->nonusedmem_p,
			    stats->usedmem, stats->usedmem_p, stats->nonusedmem_byu,
			    stats->nonusedmem_p_byu, stats->usedmem_byu,
			    stats->usedmem_p_byu
			    );
	}
}

int mem_dbg_is_freeded(const void *addr)
{
	uint8_t *ptr = (uint8_t *) addr - sizeof(uint64_t);
	int page_ind = Page_find_chks_page(ptr);
	
	if (page_ind == -1)
		return 1;
	
	Page_T page = PAGEPTR_FETCH_PAGE_T(pageptrs.buff[page_ind]);
	Chk_T chk = CHKPTR_FETCH_CHK_T(ptr);
	
	if (chk.capacity == 0 || chk.size >= page.size)
		RAISE(ExceptCorruptedAddr, "The reserved addr has an invalid capacity");

	if (page.available <= page.ptr)
		return 1;

	if (Chk_verify_checksum(&chk) == 1)
		return 1;
	
	return 0;
}


void mem_dbg_verify_ds_integrity(void)
{
	/* Verifies the integrity of the used dataestrcutres to catch something weird happend
	 */
	
	int damage;

	damage = Heap_verify_integrity(&heap_pages, &Page_capacity_cmp);
	if (damage)
		RAISE(ExceptCorruptedHeapDS, "Fatal error probably because of a buffer overflow issue");

	damage = Heap_verify_integrity(&heap_free_chunks, &Chk_capacity_cmp);
	if (damage)
		RAISE(ExceptCorruptedHeapDS, "Fatal error probably because of a buffer overflow issue");
}

