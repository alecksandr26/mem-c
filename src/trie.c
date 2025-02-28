#include <except/assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "trie.h"
#include "stack.h"

/* TODO: Create the exceptions and investigate how to save more memory in the trie */

// Allocate the Trie, and initialized its first root node
// Allocating 4210688 about 4 megabytes, but offers about 1 million of
// addresses

/* 67633152 bytes around 64.5 megabytes */

// NOTE: Change this logic to avoid wasting those page ptr bytes
static TrieNode trie[TRIE_CAPACITY] = {{
    .children =
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1},
    .page = NULL,
}};

static uint32_t trie_size = 1;

static int32_t new_trie_node(void)
{
	int32_t new_trie_index = -1;
	if (Stack_size() > 0)
		new_trie_index = Stack_pop();
	else {
		assert(trie_size < TRIE_CAPACITY, "Not enough space in the trie");
		new_trie_index = trie_size++;
	}

	assert(new_trie_index > 0);
	
	TrieNode *node = &trie[new_trie_index];	
	memset(node->children, -1, sizeof(node->children));
	node->page = NULL;
	return new_trie_index;
}




// All this functions run about O(1)
void Trie_map(uint64_t chkptr, uint8_t *page)
{
	assert(chkptr != 0);
	assert(page != NULL);

	// Extract the root
	TrieNode *curr = &trie[0];
	
	// Start iterating from the most significant byte
	for (int i = 7; i >= 0; i--) {
		uint8_t byte = (chkptr >> (i * 8)) & 0xFF;
		if (curr->children[byte] == -1)
			curr->children[byte] = new_trie_node();

		curr = &trie[curr->children[byte]];
	}
	
	curr->page = page;
}


// Trie_search: Returns null in case of not been allocated
uint8_t *Trie_find(uint64_t chkptr)
{
	assert(chkptr != 0);

	// Extract the root
	TrieNode *curr = &trie[0];

	// Find the map
	for (int i = 7; i >= 0; i--) {
		uint8_t byte = (chkptr >> (i * 8)) & 0xFF;
		if (curr->children[byte] == -1)
			return NULL;
		curr = &trie[curr->children[byte]];
	}
	return curr->page;
}

static inline uint32_t TrieNode_num_children(TrieNode *node)
{
	uint32_t num = 0;
	for (uint32_t i = 0; i < TRIE_CHILDREN_CAPACITY; i++)
		if (node->children[i] != -1)
			num++;
	return num;
}

static inline int32_t TrieNode_get_index(TrieNode *node)
{
	return (int32_t) (node - &trie[0]);
}

void Trie_delete(uint64_t chkptr)
{
	assert(chkptr != 0);

	// Extract the root
	TrieNode *curr = &trie[0];
	TrieNode *nodes[8];	/* To capture the path of nodes */

	// Find the map
	for (int i = 7; i >= 0; i--) {
		uint8_t byte = (chkptr >> (i * 8)) & 0xFF;
		if (curr->children[byte] == -1)
			assert(false);
		curr = &trie[curr->children[byte]];

		// Capture the node
		nodes[7 - i] = curr;
	}
	
	assert(curr->page != NULL);
	curr->page = NULL;
	
	// Mark the unique nodes from the key, as available to be used
	int i = 7;
	int32_t available_index = -1;
	for (; i >= 0; i--) {
		if (TrieNode_num_children(nodes[i]) > 1)
			break;
		available_index = TrieNode_get_index(nodes[i]);
		assert(available_index > 0);
		Stack_push(available_index); /* Push the available node */
	}

	// Cut the unique path for the key, from the last available node
	if (i >= 0) {
		for (int j = 0; j < TRIE_CHILDREN_CAPACITY; j++)
			if (nodes[i]->children[j] == available_index) {
				nodes[i]->children[j] = -1;
				break;
			}
	} else {
		for (int j = 0; j < TRIE_CHILDREN_CAPACITY; j++)
			if (trie[0].children[j] == available_index) {
				trie[0].children[j] = -1;
				break;
			}
				
	}
}
