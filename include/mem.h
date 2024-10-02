#ifndef MEM_H
#define MEM_H

/* TODO: Investigate how to get a better performance in mem_find_chks_page */
/* TODO: Make better cheksum to avoid possibly overwrittes from the user */
/* TODO: Add more metadata to the chunk to combine better the chunks */
/* TODO: Add an aligned alloc (Optional) */
/* TODO: Garbage collector */
/* TODO: Areana functionality */

#include <except.h>

#define NEW(ptr) ptr = mem_alloc(sizeof(*ptr))
#define FREE(ptr) mem_free(ptr)

extern Except_T ExceptInvalidNBytes;
extern Except_T ExceptInvalidAddr;
extern Except_T ExceptCorruptedAddr;
extern Except_T ExceptOverFreededChunks;
extern Except_T ExceptFatalPageError;

extern unsigned int MEM_ALLOC_MIN_CHUNK_SIZE;

extern void *mem_alloc(unsigned long nbytes);
extern void *mem_ralloc(void *addr, unsigned long nbytes);
extern void *mem_calloc(unsigned long obj_size, unsigned long nobjs);
extern void mem_free(void *addr);

#ifndef NDEBUG
typedef struct {
	int nchks, nfreedchks, nnonfreedchks, minchksize, maxchksize;
	double avgchksize, nfreedchks_p, nnonfreedchks_p;
	int npages, minpagesize, maxpagesize, minpagecp, maxpagecp, minpagenchks, maxpagenchks;
	double avgpagesize, avgpagecp, avgpagenchks;
	int totalmem, usedmem, nonusedmem, usedmem_byu, nonusedmem_byu;
	double usedmem_p, nonusedmem_p, usedmem_p_byu, nonusedmem_p_byu;
} MemStats_T;

extern Except_T ExceptCorruptedDS;
extern int mem_dbg_is_freeded(const void *addr);
extern void mem_dbg_fetch_mem_stats(MemStats_T *stats, int verbose, int log_fd);
extern void mem_dbg_verify_ds_integrity(void);
#endif


#endif
