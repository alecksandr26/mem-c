#include <stdio.h>
#include <string.h>

/* Include the mem header interface */
#include <mem.h>

/* Since it is an insatlled version we should compiled this manually not with makefile */
/* cc example_installed.c -lmem && ./a.out */




int main(void)
{
  /* const int *arr = mem_calloc(sizeof(int), 10); */
	
  /* printf("arr: "); */
  /* for (int i = 0; i < 10; i++) */
  /* 	printf("%i, ", arr[i]); */

  /* putchar('\n'); */

  struct Person {
    int age;
    char name[255];
  };

  struct Person *ptr;
  NEW(ptr, struct Person);

  ptr->age = 10;
  strcpy(ptr->name, "Hello there");
	
  struct Person *ptr2;
  NEW(ptr2);
	
  FREE(ptr);
	
  struct Person *ptr3;
  NEW(ptr3);

  int n = 100;
  double *arr = NEW(arr, double[n]);
	
  MemStats_T stats;
  mem_dbg_fetch_mem_stats(&stats, 3, 1);
	
	
  return 0;
}


