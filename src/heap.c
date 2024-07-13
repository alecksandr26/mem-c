#include <except/assert.h>

#include "utils.h"
#include "heap.h"

extern uint8_t *start_brk;
extern uint8_t *end_brk;

void Heap_push(Heap_T *heap, const uint8_t *ptr)
{
	assert(heap, "Can't be null");
	assert(ptr, "Can't be null");
	assert(ptr > start_brk && ptr < end_brk, "Invalid ptr");
	assert(CHUNK_SIZE(ptr) <= (uint64_t) (end_brk - ptr),
	       "Invalid ptr");

	heap->buff[heap->size] = (uint8_t *) ptr;

	int cpos = heap->size;
	int ppos = PPOS(cpos);
	uint8_t **buff = heap->buff;

	while (cpos > 0 && CHUNK_CMP(buff[ppos], buff[cpos]) < 0) {
		HEAP_SWAP(buff[ppos], buff[cpos]);

		cpos = ppos;
		ppos = PPOS(cpos);
	}
	
	heap->size++;
}

uint8_t *Heap_pop(Heap_T *heap)
{
	uint8_t *ptr = heap->buff[0];
	Heap_rem(heap, 0);
	return ptr;
}

const uint8_t *Heap_top(const Heap_T *heap)
{
	return heap->buff[0];
}

int Heap_find(const Heap_T *heap, const uint8_t *ptr)
{
	assert(heap, "Can't be null");
	assert(ptr, "Can't be null");
	assert(ptr > start_brk && ptr < end_brk, "Invalid ptr");
	assert(CHUNK_SIZE(ptr) <= (uint64_t) (end_brk - ptr),
	       "Invalid ptr");

	/* TODO: Make this O(log(n)) */
	for (uint32_t i = 0; i < heap->size; i++)
		if (heap->buff[i] == ptr)
			return i;
	return -1;
}

void Heap_rem(Heap_T *heap, int i)
{
	assert(heap, "Can't be null");
	assert(i >= 0 && i < (int) heap->size, "Invalid i");

	int ppos = i;
	uint8_t **buff = heap->buff;
	buff[ppos] = buff[--heap->size];

	while (1) {
		int lpos = LPOS(ppos);
		int rpos = RPOS(ppos);

		int mpos;
		if (lpos < (int) heap->size && CHUNK_CMP(buff[lpos], buff[ppos]) > 0)
			mpos = lpos;
		else
			mpos = ppos;

		if (rpos < (int) heap->size && CHUNK_CMP(buff[rpos], buff[mpos]) > 0)
			mpos = rpos;

		if (mpos == ppos)
			break;

		HEAP_SWAP(buff[ppos], buff[mpos]);
		ppos = mpos;
	}
}




