#ifndef HEAP_H
#define HEAP_H

#define HEAP_CAPACITY 4 * 1024

#define HEAP_LPOS(X) (((X) * 2) + 1)
#define HEAP_RPOS(X) (((X) * 2) + 2)
#define HEAP_PPOS(X) (((X) - 1) / 2)

#define HEAP_SWAP(a, b) do {			\
		typeof((a)) temp = (a);		\
		(a) = (b);			\
		(b) = temp;			\
	} while (0)

typedef struct {
	void *buff[HEAP_CAPACITY];
	unsigned int size;
} Heap_T;

/*
  int (*cmp)(const void **addr1, const void **addr2)
  
  addr1 is greater than addr2 -> 1 <= cmp
  addr2 is greater than addr1 -> -1 >= cmp
  addr1 and addr2 are equal -> 0 = cmp
*/
extern void Heap_push(Heap_T *heap, const void *ptr, int (*cmp)(const void **addr1, const void **addr2));
extern void *Heap_pop(Heap_T *heap, int (*cmp)(const void **addr1, const void **addr2));
extern const void *Heap_top(const Heap_T *heap);
extern int Heap_find(const Heap_T *heap, const void *ptr, int (*cmp)(const void **addr1, const void **addr2));
extern void Heap_rem(Heap_T *heap, int i, int (*cmp)(const void **addr1, const void **addr2));
extern void Heap_verify_integrity(const Heap_T *heap, int (*cmp)(const void **addr1, const void **addr2));

#endif

