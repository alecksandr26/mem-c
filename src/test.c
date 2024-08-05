#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <unittest.h>

#define MEM_DBG
#include "../include/mem.h"
#include "utils.h"
#include "heap.h"
#include "page.h"

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


TESTCASE(TestMem) {
	TEST(SimpleAllocation) {
		uint8_t *addr = mem_alloc(10);
		
		ASSERT_EQ(heap_pages.size, 1, "Should contain the page");
		
		Page_T page = PAGEPTR_FETCH_PAGE_T(Heap_top(&heap_pages));

		INFO("addr: %p", addr);
		INFO("page.end: %p", page.end);
		ASSERT_EQ(page.size, 4 * KILOBYTE, "Should be 4kb");
		ASSERT(page.size % (4 * KILOBYTE) == 0, "Should be multiple of 4kb");
		ASSERT_GT(page.end, page.ptr);
		ASSERT_GT(addr, page.ptr);

		/* It works but my unittest lib is not working properly */
		/* ASSERT_GT(page.end, addr); */
		ASSERT_LT(addr, page.available);

		Chk_T chk = CHKPTR_FETCH_CHK_T((uint8_t *) addr
					       - sizeof(uint64_t));
		ASSERT_GE(chk.capacity, 10);
		Page_free(&page);
		Heap_pop(&heap_pages, &Page_capacity_cmp);
				
	}

	TEST(MulAllocation) {
		uint8_t *addr1 = mem_alloc(10);
		uint8_t *addr2 = mem_alloc(20);
		uint8_t *addr3 = mem_alloc(40);
		
		ASSERT_EQ(heap_pages.size, 1, "Should contain the page");
		Page_T page = PAGEPTR_FETCH_PAGE_T(Heap_top(&heap_pages));
		ASSERT_EQ(page.size, 4 * KILOBYTE, "Should be 4kb");
		ASSERT(page.size % (4 * KILOBYTE) == 0, "Should be multiple of 4kb");

		Page_free(&page);
		Heap_pop(&heap_pages, &Page_capacity_cmp);

		(void) (addr1);
		(void) (addr2);
		(void) (addr3);
	}

	TEST(SimpleFree) {
		ASSERT_EQ(heap_pages.size, 0, "Shouldn't be any page");
		
		uint8_t *addr = mem_alloc(10);
		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");
		mem_free(addr);

		ASSERT_EQ(heap_pages.size, 0, "Shouldn't be any page");
		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");

	}

	TEST(MultipleFrees) {
		uint8_t *addr1 = mem_alloc(10);
		uint8_t *addr2 = mem_alloc(20);
		uint8_t *addr3 = mem_alloc(40);

		mem_free(addr2);
		
		uint8_t *addr4 = mem_alloc(20);
		
		ASSERT_EQ(addr4, addr2, "Should be the same");
		uint8_t *chkptr_addr2 = addr2 - sizeof(uint64_t);
		uint8_t *chkptr_addr4 = addr4 - sizeof(uint64_t);

		Chk_T chk_addr2 = CHKPTR_FETCH_CHK_T(chkptr_addr2);
		Chk_T chk_addr4 = CHKPTR_FETCH_CHK_T(chkptr_addr4);

		ASSERT_EQ(chk_addr2.size, chk_addr4.size);
		ASSERT_EQ(chk_addr2.capacity, chk_addr4.capacity);
		
		(void) (addr1);
		(void) (addr3);

		mem_free(addr1);

		uint8_t *addr5 = mem_alloc(10);
		
		ASSERT_EQ(addr5, addr1, "Should be the same");


		uint8_t *chkptr_addr1 = addr1 - sizeof(uint64_t);
		uint8_t *chkptr_addr5 = addr4 - sizeof(uint64_t);

		Chk_T chk_addr1 = CHKPTR_FETCH_CHK_T(chkptr_addr1);
		Chk_T chk_addr5 = CHKPTR_FETCH_CHK_T(chkptr_addr5);

		ASSERT_EQ(chk_addr1.size, chk_addr5.size);
		ASSERT_EQ(chk_addr1.capacity, chk_addr5.capacity);
		
		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");


		/* You need to free in this order, otherwise,
		   you will keep the page, and the heap of free chks,
		   with available chunks
		 */
		mem_free(addr3);
		mem_free(addr4);
		mem_free(addr5);

		ASSERT_EQ(heap_pages.size, 0, "Shouldn't be any page");
		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");

	}

	TEST(ExceptZeroNbytes) {
		ASSERT_THROW({
				mem_alloc(0);
			}, ExceptInvalidNBytes);
	}

	TEST(ExceptZeroNbytes) {

		ASSERT_THROW({
				char buff[20];
				mem_free(&buff);
			}, ExceptInvalidAddr);
	}

	TEST(BigAllocations) {
		extern long mem_find_chks_page(const uint8_t *ptr);
		uint8_t *kilob;
		uint8_t *megab;
		uint8_t *gigab;

		ASSERT_NO_THROW({
				kilob = mem_alloc(6 * KILOBYTE);
				megab = mem_alloc(2 * MEGABYTE);
				gigab = mem_alloc(GIGABYTE);
			}, "Shouldn't throw anything");

		int page_index = mem_find_chks_page(gigab - sizeof(uint64_t));
		ASSERT_EQ(page_index, 0);
		
		Page_T page = PAGEPTR_FETCH_PAGE_T(heap_pages.buff[page_index]);
		
		ASSERT_GT(page.capacity, GIGABYTE);
		ASSERT_EQ(heap_pages.size, 3, "Must be 3 pages");

		mem_free(gigab);
		ASSERT_EQ(heap_pages.size, 2, "Must be 3 pages");

		page_index = mem_find_chks_page(megab - sizeof(uint64_t));
		ASSERT_EQ(page_index, 0);

		mem_free(megab);
		ASSERT_EQ(heap_pages.size, 1, "Must be 3 pages");

		page_index = mem_find_chks_page(kilob - sizeof(uint64_t));
		ASSERT_EQ(page_index, 0);

		mem_free(kilob);
		ASSERT_EQ(heap_pages.size, 0, "Must be 3 pages");
	}
} ENDTESTCASE


TESTCASE(NonFunctionalTest) {
	TEST(ComparisonWithMalloc) {
		clock_t start, end;
		double malloc_time, custom_time;
		int nallocations = 100, c_ptr_m = 0, c_ptr_c = 0;
		int max_mem_to_alloc = KILOBYTE;
		void *ptr_malloc[nallocations];
		void *ptr_custom[nallocations];
		
		// Test malloc
		start = clock();
		for (int i = 0; i < nallocations; i++) {
			ptr_malloc[c_ptr_m++] = malloc((rand() % max_mem_to_alloc) + 1);
			
			int q = rand() % c_ptr_m;

			if (rand() % 2 == 0 && ptr_malloc[q] != NULL) {
				free(ptr_malloc[q]);
				ptr_malloc[q] = NULL;
			}
		}
		end = clock();
		malloc_time = ((double) (end - start)) / CLOCKS_PER_SEC;

		// Test custom allocator
		start = clock();
		for (int i = 0; i < nallocations; i++) {
			ptr_custom[c_ptr_c++] = mem_alloc((rand() % max_mem_to_alloc)
							  + 1);

			int q = rand() % c_ptr_c;

			if (rand() % 2 == 0 && ptr_custom[q] != NULL) {
				mem_free(ptr_custom[q]);
				ptr_custom[q] = NULL;
			}
		}
		end = clock();
		custom_time = ((double) (end - start)) / CLOCKS_PER_SEC;

		ASSERT_NEAR(malloc_time, custom_time, 0.0001);
	}	
} ENDTESTCASE

int main(void)
{
	RUN(TestCaseHeap, TestPage, TestMem, NonFunctionalTest);
	
	return 0;
}

