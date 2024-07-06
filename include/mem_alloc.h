#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

extern void *mem_alloc(unsigned long nbytes);
extern void *mem_ralloc(void *addr, unsigned long nbytes);
extern void *mem_calloc(unsigned long obj_size, unsigned long nobjs);
extern void mem_free(void *addr);


#ifndef NDEBUG
extern void mem_dump_heap_stdout(void);
#endif


#endif






