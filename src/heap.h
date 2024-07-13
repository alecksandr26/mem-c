#ifndef HEAP_H
#define HEAP_H

#include <inttypes.h>

#define HEAP_CAPACITY 1024

#define LPOS(X) (((X) * 2) + 1)
#define RPOS(X) (((X) * 2) + 2)
#define PPOS(X) (((X) - 1) / 2)

#define HEAP_SWAP(a, b) do {			\
		typeof((a)) temp = (a);		\
		(a) = (b);			\
		(b) = temp;			\
	} while (0)

typedef struct {
	uint8_t *buff[HEAP_CAPACITY];
	uint32_t size;
} Heap_T;

extern void Heap_push(Heap_T *heap, const uint8_t *ptr);
extern uint8_t *Heap_pop(Heap_T *heap);
extern const uint8_t *Heap_top(const Heap_T *heap);
extern int Heap_find(const Heap_T *heap, const uint8_t *ptr);
extern void Heap_rem(Heap_T *heap, int i);

#endif

