#include <inttypes.h>

#include <unittest.h>

#include "../include/mem.h"

#define KILOBYTE 1024
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)

static inline uint64_t aling_to_mul_8(uint64_t n)
{
	if (n % 8 == 0)
		return n;
	return n + (8 - (n % 8));
}


TESTCASE(TestAllocation)
{
	uint8_t *ptr;
	extern uint8_t *end_brk;
	extern uint8_t *start_brk;
	
	TEST(SimpleAllocation) {
		ASSERT_EQ(start_brk, end_brk);
		ptr = mem_alloc(10);
		ASSERT_NEQ(ptr, NULL, "Shouldn't return NULL");
		ASSERT_EQ(ptr, start_brk + sizeof(uint64_t));
		ASSERT_GT(end_brk, ptr);
		mem_free(ptr);
	}

	TEST(InvalidNBytes) {
		ASSERT_EQ(start_brk, end_brk);
		ASSERT_THROW({
				ptr = mem_alloc(0);
				mem_free(ptr);
			}, ExceptInvalidNBytes,
			"Should Return Invalid n bytes");
	}


	/* TODO: Solve a really weird BUG!!!! */
	TEST(AllocALongNumberOfBytes) {
		ASSERT_EQ(start_brk, end_brk);
		EXPECT_NO_THROW({
				ptr = mem_alloc(GIGABYTE);
				mem_free(ptr);
			});

		ASSERT_EQ(start_brk, end_brk);
	}



	TEST(Bounderies) {
		ASSERT_EQ(start_brk, end_brk);
		/* mem_dbg_dump_chunks_info(); */
		ASSERT_EQ(mem_dbg_num_chunks(), 0);
		ptr = mem_alloc(MEGABYTE);
		ASSERT_EQ(MEGABYTE + sizeof(uint64_t),
			  aling_to_mul_8(MEGABYTE + sizeof(uint64_t)));
		ASSERT_EQ(((unsigned int) (end_brk - start_brk)),
			  MEGABYTE + sizeof(uint64_t));
		ASSERT_EQ(ptr + MEGABYTE, end_brk);
		ASSERT_EQ(ptr, start_brk);
		
		mem_free(ptr);
	}
	
} ENDTESTCASE;

/* TODO: Create Testcases for free  */
/* TODO: Create Testcases for the heap dst  */
/* TODO: Create non-functional testcases */

int main(void)
{
	RUN(TestAllocation);
	
	return 0;
}

