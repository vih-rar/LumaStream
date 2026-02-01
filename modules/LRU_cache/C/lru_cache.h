#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#define HASH_SIZE 100
#include <stdint.h>

// Node in the doubly linked list
typedef struct Node {
    uint32_t key;
    void* value;
    struct Node* prev;
    struct Node* next;
} Node;

// Hash map entry for collision handling
typedef struct HashNode {
    uint32_t key;
    Node* node;
    struct HashNode* next;
} HashNode;

// LRU Cache structure
typedef struct {
    uint32_t capacity;
    uint32_t size;
    Node* head;  // Most recently used
    Node* tail;  // Least recently used
    HashNode* hash_table[HASH_SIZE];
} lru_cache_t;

// ========== TODO: Implement these functions ==========

// Hash function - returns hash index for a given key
uint32_t hash(uint32_t key);

// Create a new doubly linked list node with given key and value
Node* create_node(uint32_t key, void* value);

// Create and initialize an LRU cache with specified capacity
lru_cache_t* lru_cache_create(uint32_t capacity);

// Remove a node from the doubly linked list (doesn't free memory)
void remove_node(lru_cache_t* cache, Node* node);

// Add a node to the front of the list (marks as most recently used)
void add_to_front(lru_cache_t* cache, Node* node);

// Move an existing node to the front (combine remove + add)
void move_to_front(lru_cache_t* cache, Node* node);

// Insert a key-node mapping uint32_to the hash table
void hash_insert(lru_cache_t* cache, uint32_t key, Node* node);

// Retrieve the node associated with a key from hash table
Node* hash_get(lru_cache_t* cache, uint32_t key);

// Delete a key-node mapping from the hash table and free the hash node
void hash_delete(lru_cache_t* cache, uint32_t key);

// Get value for a key from cache, returns -1 if not found
// Updates the node to most recently used
void* lru_cache_get(lru_cache_t* cache, uint32_t key);

// Insert or update a key-value pair in the cache
// Evicts least recently used item if cache is full
void lru_cache_put(lru_cache_t* cache, uint32_t key, void* value);

// Pruint32_t cache contents for debugging (head to tail)
void lru_cache_pruint32_t(lru_cache_t* cache);

// Free all memory associated with the cache
void lru_cache_free(lru_cache_t* cache);

#endif // LRU_CACHE_H