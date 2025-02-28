#include <except/assert.h>
#include "stack.h"

/* 16392 bytes around 16 kilobytes */
static Stack_T stack = {
	.size = 0,
};

void Stack_push(uint32_t val)
{
	assert(stack.size < STACK_CAPACITY, "Not enough memory in the stack");
	stack.buff[stack.size++] = val;
}

uint32_t Stack_pop(void)
{
	assert(stack.size > 0);
	return stack.buff[--stack.size];
}

uint32_t Stack_top(void)
{
	assert(stack.size > 0);
	return stack.buff[stack.size - 1];
}

size_t Stack_size(void)
{
	return stack.size;
}
