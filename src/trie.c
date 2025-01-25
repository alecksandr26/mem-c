#include <except/assert.h>
#include <stdint.h>
#include "trie.h"

// Alloc the trie, and initialized its first root node
// Allocating 4210688 about 4 mega bytes, but offers about 1 million of addresses
// 32 * 4210688 = 129 mega bytes
// NOTE: Change this logic to avoid wasting those page ptr bytes
static TrieNode trie[TRIE_CAPACITY] = {
	{
		.children = {-1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1,
			     -1, -1, -1, -1, -1, -1, -1, -1},
		.page = NULL,
	}};

static uint32_t trie_size = 1;

static int32_t new_trie_node(void)
{
	assert(trie_size < TRIE_CAPACITY, "Not enough space in the trie");
	TrieNode *node = &trie[trie_size];	
	for (uint32_t i = 0; i < TRIE_CHILDREN_CAPACITY; i++)
		node->children[i] = -1;
	node->page = NULL;
	return trie_size++;
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

// TODO: Investigate how to reuse the space from the uneeded paths
// Is possible just allocated each node into array of 8 trie nodedes,
// And then delete the trie nodes that contains just one children
void Trie_delete(uint64_t chkptr)
{
	assert(chkptr != 0);

	// Extract the root
	TrieNode *curr = &trie[0];

	// Find the map
	for (int i = 7; i >= 0; i--) {
		uint8_t byte = (chkptr >> (i * 8)) & 0xFF;
		if (curr->children[byte] == -1)
			assert(0);
		curr = &trie[curr->children[byte]];
	}
	assert(curr->page != NULL);
	curr->page = NULL;
}
