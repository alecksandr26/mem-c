#include <stdio.h>

/* Include the mem header interface */
#include <mem.h>


/* Since it is an insatlled version we should compiled this manually not with makefile */
/* cc example_installed.c -lmem */

int main(void)
{
	const int *arr = mem_calloc(sizeof(int), 10);
	
	printf("arr: ");
	for (int i = 0; i < 10; i++)
		printf("%i, ", arr[i]);

	putchar('\n');
	
	return 0;
}


