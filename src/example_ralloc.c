#include "../include/mem.h"


int main(void)
{
  void *ptr = mem_alloc(20);
  ptr = mem_ralloc(ptr, 100);
  mem_free(ptr);
  return 0;
}



