#include <stdio.h>
#include <except.h>
#include <except/assert.h>

#include "../include/mem.h"

#define N 1000

typedef struct _NodeList {
	int val;
	struct _NodeList *next;
} NodeList;

struct {
	int size;
	NodeList *head;
} linked_list = {
	.head = NULL,
	.size = 0
};


void add_linked_list_node(int val)
{
	NodeList *new_node;

	NEW(new_node);		/* Use the MACRO to alloc a new node */
	new_node->val = val;

	/* Link the elemnet */
	new_node->next = linked_list.head;
	linked_list.head = new_node;

	/* Increment the size of the linked list */
	linked_list.size++;
}


void build_linked_list(void)
{
	for (int i = 0; i < N; i++)
		add_linked_list_node(i);
	
}


void print_linked_list(void)
{
	printf("size: %i\n", linked_list.size);
	
	NodeList *curr = linked_list.head;

	while (curr) {
		printf("<addr: %p, val: %i, next: %p>", (void *) curr, curr->val, (void *) curr->next);
		printf(" -> ");
		if (!curr->next)
			printf("(nil)");
		curr = curr->next;
	}
	putchar('\n');
}


int main(void)
{

	puts("Printing the linked list info...");
	build_linked_list();
	// print_linked_list();

	puts("Printing the debug info of the allocator");
	
	MemStats_T stats;

	mem_dbg_fetch_mem_stats(&stats, 3, 1);
	
	return 0;
}


