#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#define HASH_SIZE 100

// Node in the doubly linked list
typedef struct Node {
    int key;
    void* value;
    struct Node* prev;
    struct Node* next;
} Node;

// Hash map entry for collision handling
typedef struct HashNode {
    int key;
    Node* node;
    struct HashNode* next;
} HashNode;

// LRU Cache structure
typedef struct {
    int capacity;
    int size;
    Node* head;  // Most recently used
    Node* tail;  // Least recently used
    HashNode* hash_table[HASH_SIZE];
} lru_cache_t;

// ========== TODO: Implement these functions ==========

// Hash function - returns hash index for a given key
int hash(int key);

// Create a new doubly linked list node with given key and value
Node* create_node(int key, void* value);

// Create and initialize an LRU cache with specified capacity
lru_cache_t* lru_cache_create(int capacity);

// Remove a node from the doubly linked list (doesn't free memory)
void remove_node(lru_cache_t* cache, Node* node);

// Add a node to the front of the list (marks as most recently used)
void add_to_front(lru_cache_t* cache, Node* node);

// Move an existing node to the front (combine remove + add)
void move_to_front(lru_cache_t* cache, Node* node);

// Insert a key-node mapping into the hash table
void hash_insert(lru_cache_t* cache, int key, Node* node);

// Retrieve the node associated with a key from hash table
Node* hash_get(lru_cache_t* cache, int key);

// Delete a key-node mapping from the hash table and free the hash node
void hash_delete(lru_cache_t* cache, int key);

// Get value for a key from cache, returns -1 if not found
// Updates the node to most recently used
void* lru_cache_get(lru_cache_t* cache, int key);

// Insert or update a key-value pair in the cache
// Evicts least recently used item if cache is full
void lru_cache_put(lru_cache_t* cache, int key, void* value);

// Print cache contents for debugging (head to tail)
void lru_cache_print(lru_cache_t* cache);

// Free all memory associated with the cache
void lru_cache_free(lru_cache_t* cache);

#endif // LRU_CACHE_H