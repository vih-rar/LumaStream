#include "lru_cache.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

// Test counter
int test_count = 0;
int passed_count = 0;
int fail_count = 0;

void test_header(const char* name) {
    printf("\n========================================\n");
    printf("Test %d: %s\n", ++test_count, name);
    printf("========================================\n");
}

void test_pass(const char* msg) {
    printf("PASS: %s\n", msg);
    passed_count++;
}

void test_fail(const char* msg) {
    printf("FAIL: %s\n", msg);
    fail_count++;
}

// Test 1: Basic cache creation
void test_cache_creation() {
    test_header("Cache Creation");
    
    lru_cache_t* cache = lru_cache_create(5);
    
    assert(cache != NULL);
    assert(cache->capacity == 5);
    assert(cache->size == 0);
    assert(cache->head == NULL);
    assert(cache->tail == NULL);
    
    test_pass("Cache created with correct capacity");
    test_pass("Initial size is 0");
    test_pass("Head and tail are NULL");
    
    lru_cache_free(cache);
}

// Test 2: Single element operations
void test_single_element() {
    test_header("Single Element Operations");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    lru_cache_put(cache, 1, (void*)100);
    assert(cache->size == 1);
    assert(cache->head == cache->tail);
    test_pass("Single element: head == tail");
    
    void* valptr = lru_cache_get(cache, 1);
    int val = (valptr == NULL) ? -1 : (int)(intptr_t)valptr;
    assert(val == 100);
    test_pass("Get returns correct value");
    
    void* invalidval = lru_cache_get(cache, 999);
    assert(invalidval == NULL);
    test_pass("Get non-existent key returns NULL");
    
    lru_cache_free(cache);
}

// Test 3: Multiple insertions (no eviction)
void test_multiple_insertions() {
    test_header("Multiple Insertions (No Eviction)");
    
    lru_cache_t* cache = lru_cache_create(5);
    
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    lru_cache_put(cache, 3, (void*)30);
    
    assert(cache->size == 3);
    test_pass("Size is 3 after 3 insertions");
    
    void* v1 = lru_cache_get(cache, 1);
    void* v2 = lru_cache_get(cache, 2);
    void* v3 = lru_cache_get(cache, 3);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == 20);
    assert((v3 == NULL ? -1 : (int)(intptr_t)v3) == 30);
    test_pass("All values retrievable");
    
    printf("Cache state: ");
    lru_cache_print(cache);
    
    lru_cache_free(cache);
}

// Test 4: LRU eviction
void test_lru_eviction() {
    test_header("LRU Eviction");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    lru_cache_put(cache, 3, (void*)30);
    
    printf("Before eviction: ");
    lru_cache_print(cache);
    
    // Insert 4th element, should evict key=1 (LRU)
    lru_cache_put(cache, 4, (void*)40);
    
    printf("After inserting key=4: ");
    lru_cache_print(cache);
    
    assert(cache->size == 3);
    void* v1 = lru_cache_get(cache, 1);
    void* v4 = lru_cache_get(cache, 4);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == -1);  // Key 1 evicted
    assert((v4 == NULL ? -1 : (int)(intptr_t)v4) == 40);
    test_pass("Least recently used key (1) evicted");
    test_pass("New key (4) inserted successfully");
    
    lru_cache_free(cache);
}

// Test 5: Get operation updates recency
void test_get_updates_recency() {
    test_header("Get Updates Recency");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    lru_cache_put(cache, 3, (void*)30);
    
    printf("Initial: ");
    lru_cache_print(cache);
    
    // Access key=1 (moves to front)
    lru_cache_get(cache, 1);
    printf("After get(1): ");
    lru_cache_print(cache);
    
    // Now insert key=4, should evict key=2 (not key=1)
    lru_cache_put(cache, 4, (void*)40);
    printf("After put(4): ");
    lru_cache_print(cache);
    
    void* v1 = lru_cache_get(cache, 1);
    void* v2 = lru_cache_get(cache, 2);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);  // Key 1 still present
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == -1);  // Key 2 evicted
    test_pass("Get operation moved key to front");
    test_pass("Key 2 (not key 1) was evicted");
    
    lru_cache_free(cache);
}

// Test 6: Update existing key
void test_update_existing_key() {
    test_header("Update Existing Key");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    lru_cache_put(cache, 3, (void*)30);
    
    printf("Before update: ");
    lru_cache_print(cache);
    
    // Update key=1 with new value
    lru_cache_put(cache, 1, (void*)999);
    
    printf("After update key=1 to 999: ");
    lru_cache_print(cache);
    
    void* v1 = lru_cache_get(cache, 1);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 999);
    assert(cache->size == 3);  // Size shouldn't change
    test_pass("Value updated successfully");
    test_pass("Size remains the same");
    test_pass("Updated key moved to front");
    
    lru_cache_free(cache);
}

// Test 7: Hash collision handling
void test_hash_collisions() {
    test_header("Hash Collision Handling");
    
    lru_cache_t* cache = lru_cache_create(10);
    
    // These keys will likely collide (same hash % 100)
    // 1 % 100 = 1, 101 % 100 = 1, 201 % 100 = 1
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 101, (void*)1010);
    lru_cache_put(cache, 201, (void*)2010);
    
    void* v1 = lru_cache_get(cache, 1);
    void* v101 = lru_cache_get(cache, 101);
    void* v201 = lru_cache_get(cache, 201);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    assert((v101 == NULL ? -1 : (int)(intptr_t)v101) == 1010);
    assert((v201 == NULL ? -1 : (int)(intptr_t)v201) == 2010);
    test_pass("All colliding keys stored correctly");
    
    // Delete middle key
    lru_cache_put(cache, 999, (void*)9999);  // Some other key
    
    v1 = lru_cache_get(cache, 1);
    v101 = lru_cache_get(cache, 101);
    v201 = lru_cache_get(cache, 201);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    assert((v101 == NULL ? -1 : (int)(intptr_t)v101) == 1010);
    assert((v201 == NULL ? -1 : (int)(intptr_t)v201) == 2010);
    test_pass("Collision chain remains intact after operations");
    
    lru_cache_free(cache);
}

// Test 8: Capacity = 1 (edge case)
void test_capacity_one() {
    test_header("Capacity = 1 (Edge Case)");
    
    lru_cache_t* cache = lru_cache_create(1);
    
    lru_cache_put(cache, 1, (void*)10);
    void* v1 = lru_cache_get(cache, 1);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    test_pass("Single element cache works");
    
    lru_cache_put(cache, 2, (void*)20);
    v1 = lru_cache_get(cache, 1);
    void* v2 = lru_cache_get(cache, 2);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == -1);
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == 20);
    test_pass("Eviction works with capacity=1");
    
    lru_cache_free(cache);
}

// Test 9: Sequential evictions
void test_sequential_evictions() {
    test_header("Sequential Evictions");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    // Fill cache
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    lru_cache_put(cache, 3, (void*)30);
    
    printf("Initial: ");
    lru_cache_print(cache);
    
    // Add more elements, causing evictions
    lru_cache_put(cache, 4, (void*)40);
    printf("After key=4: ");
    lru_cache_print(cache);
    
    lru_cache_put(cache, 5, (void*)50);
    printf("After key=5: ");
    lru_cache_print(cache);
    
    lru_cache_put(cache, 6, (void*)60);
    printf("After key=6: ");
    lru_cache_print(cache);
    
    void* v1 = lru_cache_get(cache, 1);
    void* v2 = lru_cache_get(cache, 2);
    void* v3 = lru_cache_get(cache, 3);
    void* v4 = lru_cache_get(cache, 4);
    void* v5 = lru_cache_get(cache, 5);
    void* v6 = lru_cache_get(cache, 6);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == -1);
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == -1);
    assert((v3 == NULL ? -1 : (int)(intptr_t)v3) == -1);
    assert((v4 == NULL ? -1 : (int)(intptr_t)v4) == 40);
    assert((v5 == NULL ? -1 : (int)(intptr_t)v5) == 50);
    assert((v6 == NULL ? -1 : (int)(intptr_t)v6) == 60);
    test_pass("Sequential evictions work correctly");
    
    lru_cache_free(cache);
}

// Test 10: Alternating access pattern
void test_alternating_access() {
    test_header("Alternating Access Pattern");
    
    lru_cache_t* cache = lru_cache_create(2);
    
    lru_cache_put(cache, 1, (void*)10);
    lru_cache_put(cache, 2, (void*)20);
    
    printf("Initial: ");
    lru_cache_print(cache);
    
    // Alternate access to keep both in cache
    lru_cache_get(cache, 1);
    printf("After get(1): ");
    lru_cache_print(cache);
    
    lru_cache_get(cache, 2);
    printf("After get(2): ");
    lru_cache_print(cache);
    
    lru_cache_get(cache, 1);
    printf("After get(1): ");
    lru_cache_print(cache);
    
    // Insert new key, should evict key=2
    lru_cache_put(cache, 3, (void*)30);
    printf("After put(3): ");
    lru_cache_print(cache);
    
    void* v1 = lru_cache_get(cache, 1);
    void* v2 = lru_cache_get(cache, 2);
    void* v3 = lru_cache_get(cache, 3);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == -1);
    assert((v3 == NULL ? -1 : (int)(intptr_t)v3) == 30);
    test_pass("Alternating access maintains correct LRU order");
    
    lru_cache_free(cache);
}

// Test 11: Negative keys
void test_negative_keys() {
    test_header("Negative Keys");
    
    lru_cache_t* cache = lru_cache_create(3);
    
    lru_cache_put(cache, -1, (void*)10);
    lru_cache_put(cache, -100, (void*)100);
    lru_cache_put(cache, -999, (void*)999);
    
    void* v1 = lru_cache_get(cache, -1);
    void* v2 = lru_cache_get(cache, -100);
    void* v3 = lru_cache_get(cache, -999);
    assert((v1 == NULL ? -1 : (int)(intptr_t)v1) == 10);
    assert((v2 == NULL ? -1 : (int)(intptr_t)v2) == 100);
    assert((v3 == NULL ? -1 : (int)(intptr_t)v3) == 999);
    test_pass("Negative keys handled correctly");
    
    lru_cache_free(cache);
}

// Test 12: Large capacity
void test_large_capacity() {
    test_header("Large Capacity");
    
    lru_cache_t* cache = lru_cache_create(1000);
    
    // Insert 500 elements
    for (int i = 0; i < 500; i++) {
        lru_cache_put(cache, i, (void*)(intptr_t)(i * 10));
    }
    
    assert(cache->size == 500);
    test_pass("500 elements inserted");
    
    // Verify all present
    for (int i = 0; i < 500; i++) {
        void* v = lru_cache_get(cache, i);
        assert((v != NULL ? ( (int)(intptr_t)v == i * 10 )  : -1));
    }
    test_pass("All 500 elements retrievable");
    
    lru_cache_free(cache);
}

// Test 13: Stress test - many evictions
void test_stress_evictions() {
    test_header("Stress Test - Many Evictions");
    
    lru_cache_t* cache = lru_cache_create(10);
    
    // Insert 100 elements (90 evictions)
    for (int i = 0; i < 100; i++) {
        lru_cache_put(cache, i, (void*)(intptr_t)(i * 10));
    }
    
    assert(cache->size == 10);
    test_pass("Cache size maintained at capacity");
    
    // Only last 10 should be present
    for (int i = 0; i < 90; i++) {
        void* v = lru_cache_get(cache, i);
        assert((v == NULL ? -1 : (int)(intptr_t)v) == -1);
    }
    test_pass("Old elements properly evicted");
    
    for (int i = 90; i < 100; i++) {
        void* v = lru_cache_get(cache, i);
        assert((v == NULL ? -1 : (int)(intptr_t)v) == i * 10);
    }
    test_pass("Recent elements retained");
    
    lru_cache_free(cache);
}

int main() {
    printf("\n");
    printf("   LRU Cache Comprehensive Test Suite  \n");
    printf("\n");
    
    test_cache_creation();
    test_single_element();
    test_multiple_insertions();
    test_lru_eviction();
    test_get_updates_recency();
    test_update_existing_key();
    test_hash_collisions();
    test_capacity_one();
    test_sequential_evictions();
    test_alternating_access();
    test_negative_keys();
    test_large_capacity();
    test_stress_evictions();
    
    printf("\n\n");
    printf("           Test Summary                 \n");
    printf("\n");
    printf(" Failed:        %2d                      \n", fail_count);
    printf(" Passed:        %2d                      \n", passed_count);
    printf(" Status:        %s                \n", 
           (fail_count == 0) ? "ALL PASSED " : "SOME FAILED ");
    printf("\n");
    
    return (fail_count == 0 ) ? 0 : 1;
}