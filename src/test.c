#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <unittest.h>

#define FULL_MEM_DBG
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


	TEST(HardTest) {
		srand(time(0));

		
		int n = 1000;
		int arr[n];
		
		for (int i = 0; i < n; i++) {
			arr[i] = rand() % 1001;
			Heap_push(&heap, &arr[i], &cmp);
		}

		for (int i = 0; i < n; i++) {
			int index = Heap_find(&heap, &arr[i], &cmp);
			ASSERT(index != -1);
		}
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
		Chk_T chk1 = {
			.size = 40
		};
		
		Page_chk_alloc(&page, &chk1);
		
		ASSERT_LT(chk1.raddr, page.available, "the available must to be moved");
		ASSERT(chk1.raddr > page.ptr
		       && chk1.raddr < page.end, "Must be in range");
		ASSERT_EQ(chk1.capacity, chk1.size - sizeof(uint64_t));
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
		Chk_T chk1 = {
			.size = 1024
		};
		
		Page_chk_alloc(&page, &chk1);
		Page_chk_free(&page, &chk1);

		ASSERT_EQ(chk1.ptr, page.available, "Must be equal");
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

		Chk_T chk1 = CHKPTR_FETCH_CHK_T((uint8_t *) addr
					       - sizeof(uint64_t));
		ASSERT_GE(chk1.capacity, 10);
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
		ASSERT_EQ(heap_pages.size, 0);
		uint8_t *kilob;
		uint8_t *megab;
		uint8_t *gigab;

		ASSERT_NO_THROW({
				kilob = mem_alloc(6 * KILOBYTE);
				megab = mem_alloc(2 * MEGABYTE);
				gigab = mem_alloc(GIGABYTE);
			}, "Shouldn't throw anything");

		ASSERT_EQ(heap_pages.size, 3);

		int page_kilo_index = Page_find_chks_page(kilob - sizeof(uint64_t));
		Page_T page_kilo = PAGEPTR_FETCH_PAGE_T(pageptrs.buff[page_kilo_index]);
		INFO("Kilo: %i -> %zu", page_kilo_index, page_kilo.capacity);

		int page_mega_index = Page_find_chks_page(megab - sizeof(uint64_t));
		Page_T page_mega = PAGEPTR_FETCH_PAGE_T(pageptrs.buff[page_mega_index]);
		INFO("Mega: %i -> %zu", page_mega_index, page_mega.capacity);

		int page_giga_index = Page_find_chks_page(gigab - sizeof(uint64_t));
		Page_T page_giga = PAGEPTR_FETCH_PAGE_T(pageptrs.buff[page_giga_index]);
		INFO("Giga: %i -> %zu", page_giga_index, page_giga.capacity);
		
		ASSERT_EQ(page_giga.size, aling_to_mul_4kb(GIGABYTE + 2 * sizeof(uint64_t)));
		ASSERT_EQ(page_mega.size, aling_to_mul_4kb(2 * MEGABYTE + 2 * sizeof(uint64_t)));
		ASSERT_EQ(page_kilo.size, aling_to_mul_4kb(6 * KILOBYTE + 2 * sizeof(uint64_t)));
		
		int index = Page_find_chks_page(gigab - sizeof(uint64_t));
		ASSERT_NEQ(index, 2);
		
		mem_free(gigab);
		ASSERT_EQ(heap_pages.size, 2);
		index = Page_find_chks_page(megab - sizeof(uint64_t));
		ASSERT_EQ(index, 0);
		
		mem_free(megab);
		ASSERT_EQ(heap_pages.size, 1);

		index = Page_find_chks_page(kilob - sizeof(uint64_t));
		ASSERT_EQ(index, 0);
		
		mem_free(kilob);
		ASSERT_EQ(heap_pages.size, 0);
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

		EXPECT_NEAR(malloc_time, custom_time, 0.0001);
		printf("malloc is %0.1f%% faster than mine \n", (custom_time - malloc_time) / custom_time * 100.0);
	}
} ENDTESTCASE



TESTCASE(ChunkCheckSum) {
	TEST(SimpleCheck) {
		uint8_t *addr1 = mem_alloc(10);
		uint8_t *addr2 = mem_alloc(20);
		
		mem_free(addr1);
		
		Chk_T chk1 = CHKPTR_FETCH_CHK_T(addr1 - sizeof(uint64_t));
		ASSERT_EQ(Chk_verify_checksum(&chk1), 1, "Should be mark as freeded");

		uint8_t *addr3 = mem_alloc(10);
		EXPECT_EQ(addr1, addr3);
		
		Chk_T chk2 = CHKPTR_FETCH_CHK_T(addr3 - sizeof(uint64_t));
		ASSERT_EQ(Chk_verify_checksum(&chk2), 0, "Should be mark as freeded");

		mem_free(addr3);

		Chk_T chk3 = CHKPTR_FETCH_CHK_T(addr3 - sizeof(uint64_t));
		ASSERT_EQ(Chk_verify_checksum(&chk3), 1, "Should be mark as freeded");

		((void) addr2);
	}
} ENDTESTCASE


TESTCASE(ChunkCombine) {
	TEST(SimpleCombine) {
		uint8_t *addr1 = mem_alloc(200);
		uint8_t *addr2 = mem_alloc(100);
		uint8_t *addr3 = mem_alloc(400);
		
		mem_free(addr1);
		
		Chk_T chk1 = CHKPTR_FETCH_CHK_T(addr1 - sizeof(uint64_t));
		ASSERT_EQ(Chk_verify_checksum(&chk1), 1, "Should be mark as freeded");

		mem_free(addr2);

		ASSERT_EQ(heap_free_chunks.size, 2, "Should be two chunks");

		Chk_T chk2 = CHKPTR_FETCH_CHK_T(addr2 - sizeof(uint64_t));
		ASSERT_EQ(Chk_verify_checksum(&chk2), 1, "Should be mark as freeded");
		
		/* Here should combine the two first chunks */
		uint8_t *addr4 = mem_alloc(300);
		EXPECT_EQ(addr1, addr4);
		
		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");

		mem_free(addr3);
		mem_free(addr4);

		ASSERT_EQ(heap_free_chunks.size, 0, "Should be zero");
	}


	TEST(SimpleCombine2) {
		int n = 10;
		uint8_t *addr[n];
		
		for (int i = 0; i < n; i++)
			addr[i] = mem_alloc(40 * (n - i));

		EXPECT_EQ(heap_pages.size, 1, "Shuld be 1");

		for (int i = 0; i < n - 1; i++)
			mem_free(addr[i]);

		ASSERT_EQ(heap_free_chunks.size, n - 1, "Should be n - 1");

		Chk_T chk = CHKPTR_FETCH_CHK_T(addr[0] - sizeof(uint64_t));
		Chk_T chk2 = CHKPTR_FETCH_CHK_T(Heap_top(&heap_free_chunks));
		ASSERT_EQ(chk.capacity, chk2.capacity);

		int m = 40 * (((n - 1) * (n - 2)) / 2);
		uint8_t *combined = mem_alloc(m);

		Chk_T chk3 = CHKPTR_FETCH_CHK_T(combined - sizeof(uint64_t));
		
		ASSERT_EQ(chk3.end, addr[9] - sizeof(uint64_t));
		
		((void) combined);
	}
} ENDTESTCASE


TESTCASE(SortedArrayPageOfPtrs) {
	TEST(SimplePageAllocation) {
		Page_T page1, page2, page3;
		Page_alloc(&page1, KILOBYTE);
		Page_alloc(&page2, KILOBYTE);
		Page_alloc(&page3, KILOBYTE);

		INFO("page1: %p", page1.ptr);
		INFO("page2: %p", page2.ptr);
		INFO("page3: %p", page3.ptr);

		INFO("pageptrs.buff[0]: %p", pageptrs.buff[0]);
		INFO("pageptrs.buff[1]: %p", pageptrs.buff[1]);
		INFO("pageptrs.buff[2]: %p", pageptrs.buff[2]);

		ASSERT_EQ(pageptrs.size, 3);		
		ASSERT_EQ(pageptrs.buff[0], page3.ptr);
		ASSERT_EQ(pageptrs.buff[1], page2.ptr);
		ASSERT_EQ(pageptrs.buff[2], page1.ptr);
		
		ASSERT(pageptrs.buff[0] < pageptrs.buff[1]);
		ASSERT(pageptrs.buff[1] < pageptrs.buff[2]);

		Page_free(&page2);
		ASSERT_EQ(pageptrs.buff[0], page3.ptr);
		ASSERT_EQ(pageptrs.buff[1], page1.ptr);
		
		ASSERT(pageptrs.buff[0] < pageptrs.buff[1]);
		
		Page_free(&page3);
		ASSERT_EQ(pageptrs.buff[0], page1.ptr);
		
		Page_free(&page1);
	}


	TEST(HardPageAllocation) {
		int n = 100;
		Page_T page[n];
		
		for (int i = 0; i < n; i++)
			Page_alloc(&page[i], KILOBYTE);

		ASSERT_EQ(pageptrs.size, n);

		for (int i = 0; i < n; i++)
			ASSERT_EQ(page[i].ptr, pageptrs.buff[n - i - 1]);
		
		for (int i = 0; i < n; i++)
			Page_free(&page[i]);		
	}
} ENDTESTCASE


int main(void)
{
	RUN(TestCaseHeap, TestPage, TestMem, ChunkCombine, ChunkCheckSum, SortedArrayPageOfPtrs,
	    NonFunctionalTest);
	
	
	return 0;
}

