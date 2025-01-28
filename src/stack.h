#ifndef STACK_INCLUDED
#define STACK_INCLUDED

#include <stdint.h>
#include <stddef.h>

#include "heap.h"

#define STACK_CAPACITY HEAP_CAPACITY

typedef struct {
	uint32_t buff[STACK_CAPACITY];
	size_t size;
} Stack_T;

extern void Stack_push(uint32_t val);
extern uint32_t Stack_pop(void);
extern uint32_t Stack_top(void);
extern size_t Stack_size(void);

#endif
