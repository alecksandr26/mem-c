#include <stdio.h>
#include "include/mem_alloc.h"
#include <inttypes.h>

extern uint8_t *start_brk;
extern uint8_t *end_brk;

int main(void)
{
	void *addr = mem_alloc(10);
	void *addr2 = mem_alloc(20);
	void *addr3 = mem_alloc(30);


	mem_dump_heap_stdout();

	mem_free(addr3);

	mem_dump_heap_stdout();
	
	return 0;
}




