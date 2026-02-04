#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <except.h>
#include <except/assert.h>
#include <stddef.h>
#include <stdio.h>
#include <immintrin.h>
#include <cpuid.h>

#include "chk.h"
#include "utils.h"
#include "heap.h"
#include "page.h"

// Cache sizes
#define L1_CACHE_HALF (16 * 1024)
#define L2_CACHE_HALF (128 * 1024)

int default_log_fd = 1;
Except_T ExceptInvalidNBytes = INIT_EXCEPT_T("Invalid number of bytes");
Except_T ExceptInvalidAddr = INIT_EXCEPT_T("Invalid address");
Except_T ExceptCorruptedAddr = INIT_EXCEPT_T("Corrupted address");

// Static globals - initialized once at startup
static int cpu_has_avx2 = -1;
static int cpu_has_sse2 = -1;

__attribute__((constructor))	/* It will run this func before the main */
static void init_cpu_features(void) {
  unsigned int eax, ebx, ecx, edx;
  
  __get_cpuid(7, &eax, &ebx, &ecx, &edx);
  cpu_has_avx2 = (ebx & bit_AVX2) != 0;
  
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  cpu_has_sse2 = (edx & bit_SSE2) != 0;
}


void *mem_alloc(unsigned long nbytes)
{
  if (nbytes == 0)
    RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");

  // Align with the size of the metadata
  nbytes = aling_to_mul_8(nbytes + 2 * sizeof(uint64_t));
  nbytes = MAX((uint32_t) CHK_MIN_CHUNK_SIZE, nbytes);
  Chk_T chk = {
    .size = nbytes
  };

  if (heap_free_chunks.size > 0) {
    Chk_T top = CHKPTR_FETCH_CHK_T(Heap_top(&heap_free_chunks));
    Chk_combine_with_freeded_neighbor(&top, PAGEPTR_AVAILABLE_ADDR(top.pageptr));
  }
	
  if (heap_free_chunks.size > 0
      && CHKPTR_SIZE(Heap_top(&heap_free_chunks)) >= nbytes) {
    uint8_t *frag_chkptr = Heap_pop(&heap_free_chunks, &Chk_capacity_cmp);
    Chk_T frag_chk = CHKPTR_FETCH_CHK_T(frag_chkptr);
    frag_chk.capacity -= chk.size;
    frag_chk.size -= chk.size;
    if (frag_chk.size >= CHK_MIN_CHUNK_SIZE) {
      /* TODO: Use the lower addressed chk and push to free the higher */
      chk.ptr = (uint8_t *) frag_chk.raddr + frag_chk.capacity;
      chk.capacity = chk.size - 2 * sizeof(uint64_t);
      chk.pageptr = frag_chk.pageptr;

      // Store the metadata for the new chunk
      *((uint64_t *) chk.ptr) = chk.capacity;
      *((uint64_t *) chk.ptr + 1) = (uint64_t) chk.pageptr;
      *((uint64_t *) frag_chk.ptr) = frag_chk.capacity;
			
      chk.raddr = chk.ptr + 2 * sizeof(uint64_t);
      
      assert(CHKPTR_CAPACITY(chk.ptr) == (uint32_t) chk.capacity);
      assert(CHKPTR_PAGEPTR(chk.ptr) == frag_chk.pageptr);
      assert(CHKPTR_CAPACITY(frag_chk.ptr) == (uint32_t) \
	     frag_chk.capacity);
			
      Chk_put_checksum(&frag_chk);
      Heap_push(&heap_free_chunks, frag_chk.ptr, &Chk_capacity_cmp);
			
      Chk_rem_checksum(&chk);
      /* Since it is a new chunk we need to map it */
      /* Trie_map((uint64_t) chk.raddr, Trie_find((uint64_t) frag_chk.raddr)); */
      return (void *) chk.raddr;
    }

    Chk_rem_checksum(&frag_chk);
    return (void *) frag_chk.raddr;
  }

	
  Page_T page;
  if (heap_pages.size > 0
      && PAGEPTR_CAPACITY(Heap_top(&heap_pages)) > nbytes) {
    uint8_t *top_page_ptr = Heap_pop(&heap_pages, &Page_capacity_cmp);
    Page_T top_page = PAGEPTR_FETCH_PAGE_T(top_page_ptr);
    page = top_page;
  } else {
    Page_alloc(&page, nbytes);
  }

  Page_chk_alloc(&page, &chk);
	
  Heap_push(&heap_pages, page.ptr, &Page_capacity_cmp);
  
  assert(chk.capacity > 0 && chk.capacity < chk.size);
  assert(chk.ptr > page.ptr);
  assert(chk.raddr < page.end);
  assert(chk.raddr < page.available);
  assert(heap_pages.size > 0);

  return (void *) chk.raddr;
}

void *mem_copy(void * restrict dest, const void * restrict src, unsigned long nbytes)
{
  uint8_t *d = (uint8_t *) dest;
  uint8_t *s = (uint8_t *) src;
  
  // ============================================================
  // TINY COPIES: 0-32 bytes (bit testing, no branches!)
  // ============================================================
  if (__builtin_expect(nbytes < 32, 0)) {
    // Bit testing technique from glibc
    if (nbytes & 1) {
      *d = *s;
      d++;
      s++;
    }
    if (nbytes & 2) {
      *((uint16_t *) d) = *((uint16_t *) s);
      d += 2;
      s += 2;
    }
    if (nbytes & 4) {
      *((uint32_t *) d) = *((uint32_t *) s);
      d += 4;
      s += 4;
    }
    if (nbytes & 8) {
      *((uint64_t *) d) = *((uint64_t *) s);
      d += 8;
      s += 8;
    }
    if (nbytes & 16) {
      *((uint64_t *) d) = *((uint64_t *) s);
      *((uint64_t *) (d + 8)) = *((uint64_t *) (s + 8));
    }
    return dest;
  }
  
  // ============================================================
  // ALIGNMENT: Align source to 8-byte boundary
  // ============================================================
  {
    uintptr_t src_addr = (uintptr_t) s;
    unsigned long misalign = src_addr & 7;
    
    if (misalign) {
      unsigned long align_bytes = 8 - misalign;
      
      // Manual unroll alignment
      switch (align_bytes) {
      case 7:
	d[6] = s[6];
	__attribute__((fallthrough));
      case 6:
	d[5] = s[5];
	__attribute__((fallthrough));
      case 5:
	d[4] = s[4];
	__attribute__((fallthrough));
      case 4:
	*((uint32_t *) d) = *((uint32_t *) s); 
	d += 4;
	s += 4;
	break;
      case 3:
	d[2] = s[2];
	__attribute__((fallthrough));
      case 2:
	*((uint16_t *) d) = *((uint16_t *)s); 
	d += 2;
	s += 2;
	break;
      case 1:
	*d++ = *s++;
	break;
      }
      
      nbytes -= align_bytes;
    }
  }
  
  unsigned long i = 0;
  
  // ============================================================
  // HUGE COPIES: Non-temporal stores (bypass cache)
  // ============================================================
  if (__builtin_expect(nbytes >= L2_CACHE_HALF, 0)) {
    if (cpu_has_avx2) {
      unsigned long nt_limit = nbytes & ~127UL;  // Clear lower 7 bits
      
      // Prefetch distance
#define PREFETCH_DISTANCE 512
      
      for (i = 0; i < nt_limit; i += 128) {
	// Prefetch ahead
	__builtin_prefetch(s + i + PREFETCH_DISTANCE, 0, 0);
        
	// Load 128 bytes
	__m256i v0 = _mm256_loadu_si256((__m256i *)(s + i));
	__m256i v1 = _mm256_loadu_si256((__m256i *)(s + i + 32));
	__m256i v2 = _mm256_loadu_si256((__m256i *)(s + i + 64));
	__m256i v3 = _mm256_loadu_si256((__m256i *)(s + i + 96));
        
	// Non-temporal stores
	_mm256_stream_si256((__m256i *)(d + i), v0);
	_mm256_stream_si256((__m256i *)(d + i + 32), v1);
	_mm256_stream_si256((__m256i *)(d + i + 64), v2);
	_mm256_stream_si256((__m256i *)(d + i + 96), v3);
      }
      
      _mm_sfence();
      goto tail_copy;
    }
  }
  
  // ============================================================
  // LARGE COPIES: AVX2 with prefetching
  // ============================================================
  if (__builtin_expect(nbytes >= L1_CACHE_HALF, 0)) {
    if (cpu_has_avx2) {
      unsigned long avx_limit = nbytes & ~127UL;
      
      for (i = 0; i < avx_limit; i += 128) {
	__builtin_prefetch(s + i + 256, 0, 3);
        
	__m256i v0 = _mm256_loadu_si256((__m256i *)(s + i));
	__m256i v1 = _mm256_loadu_si256((__m256i *)(s + i + 32));
	__m256i v2 = _mm256_loadu_si256((__m256i *)(s + i + 64));
	__m256i v3 = _mm256_loadu_si256((__m256i *)(s + i + 96));
        
	_mm256_storeu_si256((__m256i *)(d + i), v0);
	_mm256_storeu_si256((__m256i *)(d + i + 32), v1);
	_mm256_storeu_si256((__m256i *)(d + i + 64), v2);
	_mm256_storeu_si256((__m256i *)(d + i + 96), v3);
      }
      
      // Remaining 32-byte chunks
      unsigned long final_avx = nbytes & ~31UL;
      for (; i < final_avx; i += 32) {
	__m256i v = _mm256_loadu_si256((__m256i *)(s + i));
	_mm256_storeu_si256((__m256i *)(d + i), v);
      }
      
      goto tail_copy;
    }
  }
  
  // ============================================================
  // MEDIUM COPIES: AVX2 or SSE2
  // ============================================================
  if (cpu_has_avx2 && nbytes >= 32) {
    // Process 128 bytes at a time (4x unroll)
    unsigned long avx_big = nbytes & ~127UL;
    
    for (i = 0; i < avx_big; i += 128) {
      __m256i v0 = _mm256_loadu_si256((__m256i *)(s + i));
      __m256i v1 = _mm256_loadu_si256((__m256i *)(s + i + 32));
      __m256i v2 = _mm256_loadu_si256((__m256i *)(s + i + 64));
      __m256i v3 = _mm256_loadu_si256((__m256i *)(s + i + 96));
      
      _mm256_storeu_si256((__m256i *)(d + i), v0);
      _mm256_storeu_si256((__m256i *)(d + i + 32), v1);
      _mm256_storeu_si256((__m256i *)(d + i + 64), v2);
      _mm256_storeu_si256((__m256i *)(d + i + 96), v3);
    }
    
    // Remaining 32-byte chunks
    unsigned long avx_small = nbytes & ~31UL;
    for (; i < avx_small; i += 32) {
      __m256i v = _mm256_loadu_si256((__m256i *)(s + i));
      _mm256_storeu_si256((__m256i *)(d + i), v);
    }
  } else if (cpu_has_sse2 && nbytes >= 16) {
    // SSE2 fallback with 4x unroll
    unsigned long sse_big = nbytes & ~63UL;
    
    for (i = 0; i < sse_big; i += 64) {
      __m128i v0 = _mm_loadu_si128((__m128i *)(s + i));
      __m128i v1 = _mm_loadu_si128((__m128i *)(s + i + 16));
      __m128i v2 = _mm_loadu_si128((__m128i *)(s + i + 32));
      __m128i v3 = _mm_loadu_si128((__m128i *)(s + i + 48));
      
      _mm_storeu_si128((__m128i *)(d + i), v0);
      _mm_storeu_si128((__m128i *)(d + i + 16), v1);
      _mm_storeu_si128((__m128i *)(d + i + 32), v2);
      _mm_storeu_si128((__m128i *)(d + i + 48), v3);
    }
    
    unsigned long sse_small = nbytes & ~15UL;
    for (; i < sse_small; i += 16) {
      __m128i v = _mm_loadu_si128((__m128i *)(s + i));
      _mm_storeu_si128((__m128i *)(d + i), v);
    }
  }
  
  // ============================================================
  // TAIL COPY: Handle remaining bytes
  // ============================================================
 tail_copy:
  {
    unsigned long remaining = nbytes - i;
    
    // Use overlapping copy technique for tail
    if (remaining >= 8) {
      // Copy 8-byte chunks
      while (remaining >= 16) {
	*(uint64_t *)(d + i) = *(uint64_t *)(s + i);
	*(uint64_t *)(d + i + 8) = *(uint64_t *)(s + i + 8);
	i += 16;
	remaining -= 16;
      }
      
      if (remaining >= 8) {
	*(uint64_t *)(d + i) = *(uint64_t *)(s + i);
	i += 8;
	remaining -= 8;
      }
    }
    
    // Final 0-7 bytes using bit testing
    if (remaining & 4) {
      *(uint32_t *)(d + i) = *(uint32_t *)(s + i);
      i += 4;
    }
    if (remaining & 2) {
      *(uint16_t *)(d + i) = *(uint16_t *)(s + i);
      i += 2;
    }
    if (remaining & 1) {
      d[i] = s[i];
    }
  }
  
  return dest;
}

void mem_free(void *addr)
{
  uint8_t *chkptr = (uint8_t *) addr - 2 * sizeof(uint64_t);
  Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);

  uint8_t *pageptr = chk.pageptr;
  if (pageptr == NULL)
    RAISE(ExceptInvalidAddr, "Can't free an invalid addr");

  Page_T page = PAGEPTR_FETCH_PAGE_T(pageptr);
  if (chk.capacity == 0 || chk.size >= page.size)
    RAISE(ExceptCorruptedAddr, "The reserved addr has an invalid capacity");

  if (page.available <= chk.ptr
      || Chk_verify_checksum(&chk) == 1)
    RAISE(ExceptInvalidAddr, "Address already freed");

  uint32_t page_index = Heap_find(&heap_pages, page.ptr,&Page_capacity_cmp);
  Heap_rem(&heap_pages, page_index, &Page_capacity_cmp);
  Page_chk_free(&page, &chk);

  if (page.available - 2 * sizeof(uint64_t) == page.ptr) {
    Page_free(&page);
    return;
  }

  Heap_push(&heap_pages, page.ptr, &Page_capacity_cmp);
}

void *mem_ralloc(void *addr, unsigned long nbytes)
{
  if (nbytes == 0)
    RAISE(ExceptInvalidNBytes, "Can't alloc zero bytes (nbytes = 0)");
	
  uint8_t *chkptr = (uint8_t *) addr - 2 * sizeof(uint64_t);
  uint8_t *pageptr = Page_find_chks_page(chkptr);
  if (pageptr == NULL)
    RAISE(ExceptInvalidAddr, "Can't free an invalid addr");
  Chk_T chk = CHKPTR_FETCH_CHK_T(chkptr);
	
  void *new_addr = mem_alloc(nbytes);

  uint32_t n = MIN(nbytes, (uint64_t) chk.capacity);
  mem_copy(new_addr, addr, n);
  
  mem_free(addr);
	
  return new_addr;
}

void *mem_set(void *dest, int value, unsigned long nbytes)
{
    uint8_t *d = (uint8_t *)dest;
    uint8_t byte_val = (uint8_t)value;
    
    // ============================================================
    // TINY SETS: 0-32 bytes (forward-backward technique)
    // ============================================================
    if (nbytes <= 32) {
        if (nbytes == 0) return dest;
        
        // Set first bytes
        if (nbytes >= 16) {
            uint64_t pattern = 0x0101010101010101ULL * byte_val;
            *(uint64_t *)d = pattern;
            *(uint64_t *)(d + 8) = pattern;
        } else if (nbytes >= 8) {
            uint64_t pattern = 0x0101010101010101ULL * byte_val;
            *(uint64_t *)d = pattern;
        } else if (nbytes >= 4) {
            uint32_t pattern = 0x01010101U * byte_val;
            *(uint32_t *)d = pattern;
        } else if (nbytes >= 2) {
            uint16_t pattern = 0x0101U * byte_val;
            *(uint16_t *)d = pattern;
        } else {
            *d = byte_val;
        }
        
        // Set last bytes (overlapping is OK!)
        if (nbytes > 16) {
            uint64_t pattern = 0x0101010101010101ULL * byte_val;
            *(uint64_t *)(d + nbytes - 16) = pattern;
            *(uint64_t *)(d + nbytes - 8) = pattern;
        } else if (nbytes > 8) {
            uint64_t pattern = 0x0101010101010101ULL * byte_val;
            *(uint64_t *)(d + nbytes - 8) = pattern;
        } else if (nbytes > 4) {
            uint32_t pattern = 0x01010101U * byte_val;
            *(uint32_t *)(d + nbytes - 4) = pattern;
        } else if (nbytes > 2) {
            uint16_t pattern = 0x0101U * byte_val;
            *(uint16_t *)(d + nbytes - 2) = pattern;
        } else if (nbytes > 1) {
            d[nbytes - 1] = byte_val;
        }
        
        return dest;
    }
    
    // ============================================================
    // ALIGNMENT: Align to 16-byte boundary
    // ============================================================
    {
        uintptr_t addr = (uintptr_t)d;
        unsigned long misalign = addr & 15;
        
        if (misalign) {
            unsigned long align_bytes = 16 - misalign;
            
            // Align using word copies
            for (unsigned long i = 0; i < align_bytes && i < nbytes; i++) {
                d[i] = byte_val;
            }
            
            d += align_bytes;
            nbytes -= align_bytes;
        }
    }
    
    unsigned long i = 0;
    
    // ============================================================
    // HUGE SETS: Non-temporal stores (bypass cache)
    // ============================================================
    if (__builtin_expect(nbytes >= L2_CACHE_HALF, 0)) {
        if (cpu_has_avx2) {
            // Create 256-bit pattern
            uint32_t pattern32 = 0x01010101U * byte_val;
            __m256i pattern = _mm256_set1_epi32(pattern32);
            
            unsigned long nt_limit = nbytes & ~127UL;
            
            for (i = 0; i < nt_limit; i += 128) {
                // Non-temporal stores - bypass cache!
                _mm256_stream_si256((__m256i *)(d + i), pattern);
                _mm256_stream_si256((__m256i *)(d + i + 32), pattern);
                _mm256_stream_si256((__m256i *)(d + i + 64), pattern);
                _mm256_stream_si256((__m256i *)(d + i + 96), pattern);
            }
            
            _mm_sfence();
            goto tail_set;
        }
    }
    
    // ============================================================
    // LARGE SETS: AVX2 with regular stores
    // ============================================================
    if (__builtin_expect(nbytes >= L1_CACHE_HALF, 0)) {
        if (cpu_has_avx2) {
            uint32_t pattern32 = 0x01010101U * byte_val;
            __m256i pattern = _mm256_set1_epi32(pattern32);
            
            unsigned long avx_limit = nbytes & ~127UL;
            
            for (i = 0; i < avx_limit; i += 128) {
                _mm256_storeu_si256((__m256i *)(d + i), pattern);
                _mm256_storeu_si256((__m256i *)(d + i + 32), pattern);
                _mm256_storeu_si256((__m256i *)(d + i + 64), pattern);
                _mm256_storeu_si256((__m256i *)(d + i + 96), pattern);
            }
            
            // Remaining 32-byte chunks
            unsigned long final_avx = nbytes & ~31UL;
            for (; i < final_avx; i += 32) {
                _mm256_storeu_si256((__m256i *)(d + i), pattern);
            }
            
            goto tail_set;
        }
    }
    
    // ============================================================
    // MEDIUM SETS: AVX2 or SSE2
    // ============================================================
    if (cpu_has_avx2 && nbytes >= 32) {
        uint32_t pattern32 = 0x01010101U * byte_val;
        __m256i pattern = _mm256_set1_epi32(pattern32);
        
        // 4x unrolled (128 bytes)
        unsigned long avx_big = nbytes & ~127UL;
        
        for (i = 0; i < avx_big; i += 128) {
            _mm256_storeu_si256((__m256i *)(d + i), pattern);
            _mm256_storeu_si256((__m256i *)(d + i + 32), pattern);
            _mm256_storeu_si256((__m256i *)(d + i + 64), pattern);
            _mm256_storeu_si256((__m256i *)(d + i + 96), pattern);
        }
        
        // Remaining 32-byte chunks
        unsigned long avx_small = nbytes & ~31UL;
        for (; i < avx_small; i += 32) {
            _mm256_storeu_si256((__m256i *)(d + i), pattern);
        }
    } else if (cpu_has_sse2 && nbytes >= 16) {
        // SSE2 fallback
        uint32_t pattern32 = 0x01010101U * byte_val;
        __m128i pattern = _mm_set1_epi32(pattern32);
        
        // 4x unrolled (64 bytes)
        unsigned long sse_big = nbytes & ~63UL;
        
        for (i = 0; i < sse_big; i += 64) {
            _mm_storeu_si128((__m128i *)(d + i), pattern);
            _mm_storeu_si128((__m128i *)(d + i + 16), pattern);
            _mm_storeu_si128((__m128i *)(d + i + 32), pattern);
            _mm_storeu_si128((__m128i *)(d + i + 48), pattern);
        }
        
        unsigned long sse_small = nbytes & ~15UL;
        for (; i < sse_small; i += 16) {
            _mm_storeu_si128((__m128i *)(d + i), pattern);
        }
    }
    
    // ============================================================
    // TAIL HANDLING: Remaining bytes
    // ============================================================
tail_set:
    {
        unsigned long remaining = nbytes - i;
        
        // Use word-sized sets
        if (remaining >= 8) {
            uint64_t pattern = 0x0101010101010101ULL * byte_val;
            
            while (remaining >= 16) {
                *(uint64_t *)(d + i) = pattern;
                *(uint64_t *)(d + i + 8) = pattern;
                i += 16;
                remaining -= 16;
            }
            
            if (remaining >= 8) {
                *(uint64_t *)(d + i) = pattern;
                i += 8;
                remaining -= 8;
            }
        }
        
        // Final 0-7 bytes
        if (remaining & 4) {
            uint32_t pattern = 0x01010101U * byte_val;
            *(uint32_t *)(d + i) = pattern;
            i += 4;
        }
        if (remaining & 2) {
            uint16_t pattern = 0x0101U * byte_val;
            *(uint16_t *)(d + i) = pattern;
            i += 2;
        }
        if (remaining & 1) {
            d[i] = byte_val;
        }
    }
    
    return dest;
}

void *mem_calloc(unsigned long obj_size, unsigned long nobjs)
{
  if (obj_size == 0 || nobjs == 0)
    RAISE(ExceptInvalidNBytes, "Can't alloc zero objs (obj_size = 0, or nobjs = 0)");

  void *ptr = mem_alloc(obj_size * nobjs);
  mem_set(ptr, 0, obj_size * nobjs);
	
  return ptr;
}




