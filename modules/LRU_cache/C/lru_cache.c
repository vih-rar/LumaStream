#include "lru_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// TODO: Implement hash function
// Hint: Use modulo operator with HASH_SIZE, handle negative keys
int hash(int key)
{
    return abs( key )%HASH_SIZE;
}

// TODO: Create a new node
// Hint: Allocate memory, initialize key/value, set prev/next to NULL
Node* create_node(int key, void* value) 
{
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->key = key;
    new_node->value = value;
    new_node->prev = NULL;
    new_node->next = NULL;

    return new_node;
}

// TODO: Create LRU Cache
// Hint: Allocate cache, set capacity/size, initialize head/tail to NULL
// Initialize all hash_table entries to NULL
lru_cache_t* lru_cache_create(int capacity) 
{
    lru_cache_t* cache = (lru_cache_t*)malloc(sizeof( lru_cache_t ));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    
    for( int i = 0; i<HASH_SIZE; i++ )
    {
        cache->hash_table[i] = NULL;
    }
    return cache;
}

// TODO: Remove node from doubly linked list
// Hint: Update prev node's next pointer, update next node's prev pointer
// Handle edge cases: node is head, node is tail
void remove_node(lru_cache_t* cache, Node* node) 
{
    if( node->prev )
    {
        node->prev->next = node->next;
    }
    else
    {
        cache->head = node->next;
    }

    if( node->next )
    {
        node->next->prev = node->prev;
    }
    else
    {
        cache->tail = node->prev;
    }
}

// TODO: Add node to the front (most recently used position)
// Hint: Set node->next to current head, update head->prev
// Update cache->head, handle case when list is empty (update tail too)
void add_to_front(lru_cache_t* cache, Node* node) 
{
    node->next = cache->head;
    node->prev = NULL;

    if( cache->head )
    {
        cache->head->prev = node;
    }

    cache->head = node;

    if( !cache->tail )
    {
        cache->tail = node;
    }
}

// TODO: Move node to front
// Hint: Call remove_node(), then call add_to_front()
void move_to_front(lru_cache_t* cache, Node* node)
{    
    remove_node( cache, node );
    add_to_front( cache, node );
}

// TODO: Insert into hash table
// Hint: Create HashNode, compute hash index, insert at head of chain
void hash_insert(lru_cache_t* cache, int key, Node* node)
{
    int index = hash( key );
    
    HashNode* hashnode = (HashNode*)malloc(sizeof(HashNode));
    hashnode->key = key;
    hashnode->node = node;

    hashnode->next = cache->hash_table[index];
    cache->hash_table[index] = hashnode;
}

// TODO: Get node from hash table
// Hint: Compute hash index, traverse chain to find matching key
// Return NULL if not found
Node* hash_get(lru_cache_t* cache, int key)
{
    int index = hash( key );
    HashNode* hashnode = cache->hash_table[index];

    while( hashnode )
    {
        if( hashnode->key == key )
        {
            return hashnode->node;
        }
        hashnode = hashnode->next;
    }
    return NULL;
}

// TODO: Delete from hash table
// Hint: Compute hash index, traverse chain to find key
// Update prev->next pointer, handle head of chain case, free HashNode
void hash_delete(lru_cache_t* cache, int key) 
{
    int index = hash( key );
    HashNode* hashnode = cache->hash_table[index];
    HashNode* prev = NULL;

    while( hashnode )
    {
        if( hashnode->key == key )
        {
            if( prev )
            {
                prev->next = hashnode->next;
            }
            else
            {
                cache->hash_table[index] = hashnode->next; 
            }
            free( hashnode );
            return;
        }
        prev = hashnode;
        hashnode = hashnode->next;
    }
}

// TODO: Get value from cache
// Hint: Use hash_get to find node, return -1 if not found
// Move node to front (mark as recently used), return value
void* lru_cache_get(lru_cache_t* cache, int key)
{
   Node* node = hash_get( cache, key );
   if( node == NULL )
   {
        return NULL;
   }
   move_to_front( cache, node );
   return node->value;
}

// TODO: Put key-value into cache
// Hint: Check if key exists with hash_get
// If exists: update value, move to front
// If new: create node, check if cache is full
//   If full: delete tail (LRU), remove from hash, free node, decrement size
//   Add new node to front, insert into hash, increment size
void lru_cache_put(lru_cache_t* cache, int key, void* value)
{
   Node* node = hash_get(cache, key);
   if (node)
   {
        free( node->value );
        node->value = value;  // Update value
        move_to_front(cache, node);
        return; 
   }

   node = create_node(key, value);
   if (cache->capacity == cache->size)
   {
        Node* lru = cache->tail;
        hash_delete(cache, lru->key);
        remove_node(cache, lru);
        if (lru->value) {
            free(lru->value);  // Only free if dynamically allocated
        }
        free(lru);
        cache->size--;
    }
    add_to_front(cache, node);
    hash_insert(cache, key, node);
    cache->size++;
}

// TODO: Print cache contents
// Hint: Traverse from head to tail, print key:value pairs
void lru_cache_print(lru_cache_t* cache)
{    
    printf("Cache (size=%d, capacity=%d): ", cache->size, cache->capacity);
    Node* curr = cache->head;
    while (curr) {
        printf("[%d:%d] ", curr->key, (int)(intptr_t)(curr->value));
        curr = curr->next;
    }
    printf("\n");
}

// TODO: Free cache memory
// Hint: Free all nodes in linked list
// Free all hash nodes in hash table (nested loop)
// Free cache structure itself
void lru_cache_free(lru_cache_t* cache)
{    
    printf("Freeing cache\n");
    Node* current = cache->head;

    while( current )
    {
        // printf("Freeing node with key: %d\n", current->key);
        Node* next = current->next;
        if( current->value )
        {
            // printf("Freeing value for key: %d\n", current->key);
            free( current->value );
        }
        free( current );
        current = next;
    }
    
    for( int i = 0; i<HASH_SIZE; i++ )
    {
        HashNode* hashnode = cache->hash_table[i];
        while( hashnode )
        {
            // printf("Freeing hashnode with key: %d\n", hashnode->key);
            HashNode* next = hashnode->next;
            free( hashnode );
            hashnode = next;
        }
    }
    free( cache );
}
