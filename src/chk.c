#include <except.h>
#include <except/assert.h>

#include "chk.h"
#include "heap.h"
#include "utils.h"

int CHK_MIN_CHUNK_SIZE = 40;

Except_T ExceptOverFreededChunks = INIT_EXCEPT_T("Over freeded chunks");

Heap_T heap_free_chunks = {
	.size = 0
};

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



