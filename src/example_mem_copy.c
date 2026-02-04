#include <stdio.h>
#include <except/assert.h>
#include <stdint.h>
#include <time.h>

#include "../include/mem.h"
#include "utils.h"


void example_profiling(void)
{
  clock_t start, end;
  size_t size = 5 * MEGABYTE;
  int iterations = 50;
    
  uint8_t *src = mem_alloc(size);
  uint8_t *dest1 = mem_alloc(size);
  uint8_t *dest2 = mem_alloc(size);
    
  // Test memcpy
  start = clock();
  for (int i = 0; i < iterations; i++) {
    memcpy(dest1, src, size);
  }
  end = clock();
  double memcpy_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
  // Test mem_copy
  start = clock();
  for (int i = 0; i < iterations; i++) {
    mem_copy(dest2, src, size);
  }
  end = clock();
  double custom_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
  printf("memcpy: %.4f sec | mem_copy: %.4f sec | ratio: %.1f%% \n",
	 memcpy_time, custom_time, (custom_time / memcpy_time) * 100.0);
    
  mem_free(src);
  mem_free(dest1);
  mem_free(dest2);
}

void simple_copy(void)
{
  uint8_t src[100];
  uint8_t dest[100];
    
  for (int i = 0; i < 100; i++) {
    src[i] = i;
  }
    
  mem_copy(dest, src, 100);
    
  for (int i = 0; i < 100; i++) {
    assert(dest[i] == src[i]);
  }
}

int main(void)
{
  simple_copy();
  example_profiling();
  
  return 0;
}

