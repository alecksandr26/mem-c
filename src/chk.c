#include <except.h>
#include <except/assert.h>

#include "chk.h"
#include "heap.h"
#include "utils.h"

typedef struct {
	long data[4];
} Chk_checksum_T;

int CHK_MIN_CHUNK_SIZE = 40;


Except_T ExceptOverFreededChunks = INIT_EXCEPT_T("Over freeded chunks");

Heap_T heap_free_chunks = {
	.size = 0
};

const Chk_checksum_T Chk_checksum = {{123456789L, -987654321L, 1357924680L, -246813579L}};
long Chk_checksum_res = 246913569L;


int Chk_capacity_cmp(const void **addr1, const void **addr2)
{
	assert(addr1 != NULL, "Can't null");
	assert(addr2 != NULL, "Can't null");
	
	const uint8_t *chkptr1 = (const uint8_t *) *addr1;
	const uint8_t *chkptr2 = (const uint8_t *) *addr2;

	assert(chkptr1 != NULL, "Can't null");
	assert(chkptr2 != NULL, "Can't null");
	
	Chk_T chk1 = CHKPTR_FETCH_CHK_T(chkptr1);
	Chk_T chk2 = CHKPTR_FETCH_CHK_T(chkptr2);

	assert(chk1.capacity > 0, "Can't be chunks of capacity zero");
	assert(chk2.capacity > 0, "Can't be chunks of capacity zero");
	assert(chk1.size > 0, "Can't be chunks of size zero");
	assert(chk2.size > 0, "Can't be chunks of size zero");

	return chk1.capacity - chk2.capacity;
}


void Chk_combine_with_freeded_neighbor(Chk_T *chk, const uint8_t *ava_pageptr)
{
	assert(chk != NULL && ava_pageptr != NULL);
	
	uint8_t *next_chkptr = chk->end;

	while (next_chkptr < ava_pageptr) {
		Chk_T neigh = CHKPTR_FETCH_CHK_T(next_chkptr);
		if (Chk_verify_checksum(&neigh) == 0)
			break;

		int index = Heap_find(&heap_free_chunks, next_chkptr, &Chk_capacity_cmp);
		
		assert(index != -1, "Should exist in the heap free");
		Heap_rem(&heap_free_chunks, index, &Chk_capacity_cmp);
		next_chkptr += neigh.size;
	}

	chk->capacity = (int) (next_chkptr - chk->raddr);
	chk->size = chk->capacity + sizeof(uint64_t);
	*((uint64_t *) chk->ptr) = chk->capacity;
}


void Chk_put_checksum(Chk_T *chk)
{
	assert(chk != NULL);
	assert(chk->raddr != NULL);
	
	Chk_checksum_T *ptr = (Chk_checksum_T *) chk->raddr;
	*ptr = Chk_checksum;
}

int Chk_verify_checksum(Chk_T *chk)
{
	assert(chk != NULL);

	assert(chk->raddr != NULL);
	
	Chk_checksum_T *ptr = (Chk_checksum_T *) chk->raddr;
	
	return ptr->data[0] + ptr->data[1]
		+ ptr->data[2] + ptr->data[3] == Chk_checksum_res;
}

void Chk_rem_checksum(Chk_T *chk)
{
	assert(chk != NULL);

	assert(chk->raddr != NULL);
	Chk_checksum_T *ptr = (Chk_checksum_T *) chk->raddr;

	bzero(ptr, sizeof(Chk_checksum_T));
}

