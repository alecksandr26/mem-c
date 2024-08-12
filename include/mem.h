#ifndef MEM_H
#define MEM_H

/* TODO: Refactor all the mem debug fucntions */
/* TODO: Investigate how to get a better performance in mem_find_chks_page */
/* TODO: Make better cheksum to avoid possibly overwrittes from the user */
/* TODO: Add more metadata to the chunk to combine better the chunks */
/* TODO: Add an aligned alloc (Optional) */
/* TODO: Garbage collector */
/* TODO: Areana functionality */


#include <except.h>

#define NEW(ptr) ptr = mem_alloc(sizeof(*ptr))
#define FREE(ptr) mem_free(ptr)

#ifndef NDEBUG
typedef struct {
	int nchks, nfreechks;
} MemStats_T;
#endif

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


#ifdef FULL_MEM_DBG
extern unsigned int mem_dbg_num_pages(void);
extern void mem_dbg_dump_pages_buff(void **buff, unsigned int n);
extern void mem_dbg_dump_pages_info(void);
extern unsigned int mem_dbg_num_chks_in_page(const void *page_ptr);
extern void mem_dbg_dump_info(void);
#endif



extern int mem_dbg_is_freeded(const void *addr);
extern unsigned int mem_dbg_num_chks(void);
extern void mem_dbg_dump_chks_buff(void **buff, int n);
extern void mem_dbg_dump_chks_info(void);
extern void mem_dbg_dump_stats_info(void);
#endif
#endif
