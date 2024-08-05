#include <except/assert.h>

#include "utils.h"
#include "heap.h"

void Heap_push(Heap_T *heap, const void *ptr, int (*cmp)(const void **addr1, const void **addr2))
{
	assert(heap, "Can't be null");
	assert(ptr, "Can't be null");
	assert(cmp, "Can't be null");

	heap->buff[heap->size] = (void *) ptr;

	int cpos = heap->size;
	int ppos = HEAP_PPOS(cpos);
	void **buff = heap->buff;
	
	while (cpos > 0 && (*cmp)((const void **) &buff[ppos], (const void **) &buff[cpos]) < 0) {
		HEAP_SWAP(buff[ppos], buff[cpos]);

		cpos = ppos;
		ppos = HEAP_PPOS(cpos);
	}
	
	heap->size++;
}

void *Heap_pop(Heap_T *heap, int (*cmp)(const void **addr1, const void **addr2))
{
	void *ptr = heap->buff[0];
	Heap_rem(heap, 0, cmp);
	return ptr;
}

const void *Heap_top(const Heap_T *heap)
{
	assert(heap->size);
	return heap->buff[0];
}

int Heap_find(const Heap_T *heap, const void *ptr, int (*cmp)(const void **addr1, const void **addr2))
{
	assert(heap, "Can't be null");
	assert(ptr, "Can't be null");

	/* TODO: Make this O(log(n)) */
	for (uint32_t i = 0; i < heap->size; i++)
		if (heap->buff[i] == ptr)
			return i;
	return -1;
}

void Heap_rem(Heap_T *heap, int i, int (*cmp)(const void **addr1, const void **addr2))
{
	assert(heap, "Can't be null");
	assert(i >= 0 && i < (int) heap->size, "Invalid i");
	assert(cmp, "Can't be null");

	int ppos = i;
	void **buff = heap->buff;
	buff[ppos] = buff[--heap->size];

	while (1) {
		int lpos = HEAP_LPOS(ppos);
		int rpos = HEAP_RPOS(ppos);

		int mpos;
		if (lpos < (int) heap->size && (*cmp)((const void **) &buff[lpos], (const void **) &buff[ppos]) > 0)
			mpos = lpos;
		else
			mpos = ppos;

		if (rpos < (int) heap->size && (*cmp)((const void **) &buff[rpos], (const void **) &buff[mpos]) > 0)
			mpos = rpos;

		if (mpos == ppos)
			break;

		HEAP_SWAP(buff[ppos], buff[mpos]);
		ppos = mpos;
	}
}
