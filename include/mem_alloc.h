#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

extern unsigned int MEM_ALLOC_MIN_CHUNK_SIZE;

extern void *mem_alloc(unsigned long nbytes);
extern void *mem_ralloc(void *addr, unsigned long nbytes);
extern void *mem_calloc(unsigned long obj_size, unsigned long nobjs);
extern void mem_free(void *addr);

#ifndef NDEBUG
/* TODO: Create func int mem_dbg_is_freeded(void *addr) */
/* TODO: Create func unsigned int mem_dbg_num_chunks(void) */
/* TODO: Create func void mem_dbg_dump_chunks_buff(void **buff, int n) */
extern void mem_dbg_dump_chunks_info(void);
#endif

#endif






