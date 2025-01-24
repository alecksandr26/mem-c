#ifndef TRIE_H
#define TRIE_H

// TODO: Map ptr 64 bit -> 64 bit pointer
#include <stdint.h>

#define TRIE_CAPACITY 4 * 1024
#define TRIE_CHILDREN_CAPACITY 255


typedef struct {
	int32_t children[TRIE_CHILDREN_CAPACITY];
	uint8_t *page;
} TrieNode;

extern void Trie_insert(uint64_t chkptr, uint8_t *page);

// Trie_search: Returns null in case of not been allocated
extern uint8_t *Trie_search(uint64_t chkptr);
extern void Trie_delete(uint64_t chkptr);

#endif //TRIE_H
