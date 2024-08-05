#ifndef MEM_H
#define MEM_H

#include <except.h>

extern Except_T ExceptInvalidNBytes;
extern Except_T ExceptInvalidAddr;
extern Except_T ExceptCorruptedAddr;
extern Except_T ExceptOverFreededChunks;

extern unsigned int MEM_ALLOC_MIN_CHUNK_SIZE;

extern void *mem_alloc(unsigned long nbytes);
extern void *mem_ralloc(void *addr, unsigned long nbytes);
extern void *mem_calloc(unsigned long obj_size, unsigned long nobjs);
extern void mem_free(void *addr);

#ifndef NDEBUG
#ifdef MEM_DBG
extern unsigned int mem_dbg_num_pages(void);
extern void mem_dbg_dump_pages_buff(void **buff, unsigned int n);
extern void mem_dbg_dump_pages_info(void);
extern unsigned int mem_dbg_num_chks_in_page(const void *page_ptr);
extern void mem_dbg_dump_info(void);
#endif

extern int mem_dbg_is_freeded(void *addr);
extern unsigned int mem_dbg_num_chks(void);
extern void mem_dbg_dump_chks_buff(void **buff, int n);
extern void mem_dbg_dump_chks_info(void);
#endif
#endif






