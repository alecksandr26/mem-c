#include <except/assert.h>

#include "utils.h"
#include "heap.h"

static int Heap_find_recursive(const Heap_T *heap, const void *ptr, int (*cmp)(const void **addr1, const void **addr2), int curr)
{
	/* Inorder search */
	if ((int) heap->size > curr && cmp((const void **) &heap->buff[curr], &ptr) >= 0) {
		if (heap->buff[curr] == ptr)
			return curr;
		
		int lpos = HEAP_LPOS(curr);
		int lres = Heap_find_recursive(heap, ptr, cmp, lpos);

		if (lres != -1)
			return lres;

		
		int rpos = HEAP_RPOS(curr);
		int rres = Heap_find_recursive(heap, ptr, cmp, rpos);

		if (rres != -1)
			return rres;
	}
	
	return -1;
}


static int Heap_verify_integrity_by_pos(const Heap_T *heap, int cpos,
				  int (*cmp)(const void **addr1, const void **addr2))
{
	assert(heap, "Can't be null");
	assert(cmp, "Can't be null");
	assert(cpos >= 0 && cpos < (int) heap->size, "Invailed pos");

	while (cpos > 0)  {
		int ppos = HEAP_PPOS(cpos);
		int c = (*cmp)((const void **) &heap->buff[ppos], (const void **) &heap->buff[cpos]);
		if (c < 0)
			return 1;
		cpos = ppos;
	}

	return 0;
}

int Heap_verify_integrity(const Heap_T *heap, int (*cmp)(const void **addr1, const void **addr2))
{
	for (int i = 0; i < (int) heap->size; i++)
		if (Heap_verify_integrity_by_pos(heap, i, cmp))
			return 1;
	return 0;
}

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

	/* Old version */
	/* for (uint32_t i = 0; i < heap->size; i++) */
	/* 	if (heap->buff[i] == ptr) */
	/* 		return i; */
		
	return Heap_find_recursive(heap, ptr, cmp, 0);
}

void Heap_rem(Heap_T *heap, int i, int (*cmp)(const void **addr1, const void **addr2))
{
	assert(heap, "Can't be null");
	assert(i >= 0 && i < (int) heap->size, "Invalid heap index");
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

	if (ppos == i) {
		int cpos = ppos;
		ppos = HEAP_PPOS(cpos);		
		while (cpos > 0 && (*cmp)((const void **) &buff[cpos], (const void **) &buff[ppos]) > 0)  {
			HEAP_SWAP(buff[ppos], buff[cpos]);
			cpos = ppos;
			ppos = HEAP_PPOS(cpos);			
		}
	}
}
