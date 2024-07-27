#include <inttypes.h>

#include <unittest.h>

#include "../include/mem.h"
#include "utils.h"
#include "heap.h"
#include "page.h"


/* TESTCASE(TestAllocation) */
/* { */
/* 	uint8_t *ptr; */
/* 	extern uint8_t *end_brk; */
/* 	extern uint8_t *start_brk; */
	
/* 	TEST(SimpleAllocation) { */
/* 		ASSERT_EQ(start_brk, end_brk); */
/* 		ptr = mem_alloc(10); */
/* 		ASSERT_NEQ(ptr, NULL, "Shouldn't return NULL"); */
/* 		ASSERT_EQ(ptr, start_brk + sizeof(uint64_t)); */
/* 		ASSERT_GT(end_brk, ptr); */
/* 		mem_free(ptr); */
/* 	} */

/* 	TEST(InvalidNBytes) { */
/* 		ASSERT_EQ(start_brk, end_brk); */
/* 		ASSERT_THROW({ */
/* 				ptr = mem_alloc(0); */
/* 				mem_free(ptr); */
/* 			}, ExceptInvalidNBytes, */
/* 			"Should Raise Invalid n bytes"); */
/* 	} */

	
/* 	TEST(AllocALongNumberOfBytes) { */
/* 		ASSERT_EQ(start_brk, end_brk); */
/* 		EXPECT_NO_THROW({ */
/* 				ptr = mem_alloc(GIGABYTE); */
/* 				mem_free(ptr); */
/* 			}); */
		
/* 		ASSERT_EQ(start_brk, end_brk); */
/* 	} */



/* 	TEST(Bounderies) { */
/* 		ASSERT_EQ(start_brk, end_brk); */
/* 		/\* mem_dbg_dump_chunks_info(); *\/ */
/* 		ASSERT_EQ(mem_dbg_num_chunks(), 0); */
/* 		ptr = mem_alloc(MEGABYTE); */
/* 		ASSERT_EQ(MEGABYTE + sizeof(uint64_t), */
/* 			  aling_to_mul_8(MEGABYTE + sizeof(uint64_t))); */
/* 		ASSERT_EQ(((unsigned int) (end_brk - start_brk)), */
/* 			  MEGABYTE + sizeof(uint64_t)); */
/* 		ASSERT_EQ(ptr + MEGABYTE, end_brk); */
/* 		ASSERT_EQ(ptr, start_brk); */
		
/* 		mem_free(ptr); */
/* 	} */
	
/* } ENDTESTCASE */

int cmp(const void **addr1, const void **addr2)
{
	int *num1 = (int *) *addr1;
	int *num2 = (int *) *addr2;

	return *num1 - *num2;
}

TESTCASE(TestCaseHeap) {
	Heap_T heap = { .buff = {0}, .size = 0 };

	int num1 = 5;
	int num2 = 10;
	int num3 = 15;

	Heap_push(&heap, &num1, &cmp);
	Heap_push(&heap, &num3, &cmp);
	Heap_push(&heap, &num2, &cmp);
	
	TEST(Heap_top) {
		ASSERT_EQ(*((int *) Heap_top(&heap)), 15,
			  "Must to be top");
	}
	

	TEST(Heap_find) {
		ASSERT_EQ(Heap_find(&heap, &num3, &cmp), 0,
			  "Since num3 = 15, its position must be the 0 (root)");
	}

	TEST(Heap_rem) {
		int pos = Heap_find(&heap, &num3, cmp);
		Heap_rem(&heap, pos, &cmp);

		ASSERT_EQ(*((int *) Heap_top(&heap)), 10,
			  "Must to be top");
	}
} ENDTESTCASE


TESTCASE(TestPage) {
	Page_T page;
	Page_alloc(&page, KILOBYTE);

	TEST(PageAlloc) {
		ASSERT_GE(page.capacity, KILOBYTE, "Must to have an equal or greater");
		ASSERT_GE(page.size, KILOBYTE, "Must to have an equal or greater");
		ASSERT_EQ(page.capacity + 2 * sizeof(uint64_t), page.size);
	}

	TEST(SimpleChkAlloc) {
		Chk_T chk = {
			.size = 40
		};
		
		Page_chk_alloc(&page, &chk);
		
		ASSERT_LT(chk.raddr, page.available, "the available must to be moved");
		ASSERT(chk.raddr > page.ptr
		       && chk.raddr < page.end, "Must be in range");
		ASSERT_EQ(chk.capacity, chk.size - sizeof(uint64_t));
		ASSERT_EQ(page.available, PAGEPTR_AVAILABLE_ADDR(page.ptr),
			  "Must be equal");
	}


	TEST(MultipleChkAlloc) {
		Chk_T chk1 = {
			.size = 40
		};

		Chk_T chk2 = {
			.size = 64
		};

		Chk_T chk3 = {
			.size = 72
		};
		
		Page_chk_alloc(&page, &chk1);
		Page_chk_alloc(&page, &chk2);
		Page_chk_alloc(&page, &chk3);

		ASSERT_LT(chk1.raddr, chk2.raddr);
		ASSERT_EQ(chk1.raddr, chk2.ptr - chk1.capacity);
		ASSERT_EQ(chk1.raddr, chk3.ptr - chk1.capacity - chk2.size);
		ASSERT_EQ(page.available, PAGEPTR_AVAILABLE_ADDR(page.ptr),
			  "Must be equal");
	}


	TEST(ChkFree) {
		Chk_T chk = {
			.size = 1024
		};
		
		Page_chk_alloc(&page, &chk);
		Page_chk_free(&page, &chk);

		ASSERT_EQ(chk.ptr, page.available, "Must be equal");
		ASSERT_EQ(page.available, PAGEPTR_AVAILABLE_ADDR(page.ptr),
			  "Must be equal");
		
	}

	TEST(HeapOfFreededChks) {
		Chk_T chk1 = {
			.size = 40
		};

		Chk_T chk2 = {
			.size = 64
		};

		Chk_T chk3 = {
			.size = 72
		};
		
		Page_chk_alloc(&page, &chk1);
		Page_chk_alloc(&page, &chk2);
		Page_chk_alloc(&page, &chk3);


		Page_chk_free(&page, &chk1);

		ASSERT_EQ(chk3.end, page.available, "Must be equal");
		ASSERT_EQ(heap_free_chunks.size, 1, "Must be equal to 1");
		ASSERT_EQ(Heap_top(&heap_free_chunks), chk1.ptr);


		Page_chk_free(&page, &chk2);

		ASSERT_EQ(chk3.end, page.available, "Must be equal");
		ASSERT_EQ(heap_free_chunks.size, 2, "Must be equal to 2");
		ASSERT_EQ(Heap_top(&heap_free_chunks), chk2.ptr);
		
	}

	Page_free(&page);
} ENDTESTCASE

/* TODO: Create Testcases for free  */
/* TODO: Create non-functional testcases */

int main(void)
{
	RUN(TestCaseHeap, TestPage);
	
	return 0;
}

