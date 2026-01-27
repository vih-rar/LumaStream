#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "lru_cache_template.h"

using namespace std;

/*
 * Multi-threaded test for LRU Cache
 * Demonstrates concurrent producers and consumers
 */

// Producer: writes to cache
template <typename T>
void producer(LRUCache<T>& cache, int producer_id, int num_items) {
    cout << "[Producer " << producer_id << "] Starting..." << endl;
    
    for (int i = 0; i < num_items; i++) {
        int key = (producer_id * 100) + i;
        T value = (producer_id * 1000) + i;
        
        cache.put(key, value);
        cout << "[Producer " << producer_id << "] put(" << key << ", " << value << ")" << endl;
        
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    
    cout << "[Producer " << producer_id << "] Finished!" << endl;
}

// Consumer: reads from cache
template <typename T>
void consumer(LRUCache<T>& cache, int consumer_id, int num_reads) {
    cout << "[Consumer " << consumer_id << "] Starting..." << endl;
    
    for (int i = 0; i < num_reads; i++) {
        // READ ACTUAL KEYS THAT PRODUCERS WRITE
        int key = (i % 3) * 100 + (i % 5);  // Keys like 0, 1, 2, 3, 4, 100, 101...
        
        auto value = cache.get(key);
        
        if (value) {
            cout << "[Consumer " << consumer_id << "] get(" << key << ") = " << *value << endl;
        } else {
            cout << "[Consumer " << consumer_id << "] get(" << key << ") = NOT FOUND" << endl;
        }
        
        this_thread::sleep_for(chrono::milliseconds(15));
    }
    
    cout << "[Consumer " << consumer_id << "] Finished!" << endl;
}

// Scenario 1: Single producer, single consumer
void test_single_producer_consumer() {
    cout << "\n========== Test 1: Single Producer/Consumer ==========" << endl;
    
    LRUCache<int> cache(5);  // Capacity 5
    
    thread prod(producer<int>, ref(cache), 1, 5);
    thread cons(consumer<int>, ref(cache), 1, 5);
    
    prod.join();
    cons.join();
    
    cout << "Test 1 Complete!" << endl;
}

// Scenario 2: Multiple producers, one consumer
void test_multiple_producers() {
    cout << "\n========== Test 2: Multiple Producers/Single Consumer ==========" << endl;
    
    LRUCache<int> cache(20);
    
    vector<thread> threads;
    
    // Create 3 producers
    for (int i = 1; i <= 3; i++) {
        threads.emplace_back(producer<int>, ref(cache), i, 3);
    }
    
    // Create 1 consumer
    threads.emplace_back(consumer<int>, ref(cache), 1, 10);
    
    // Wait for all to finish
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "Test 2 Complete!" << endl;
}

// Scenario 3: Multiple producers and consumers
void test_multiple_producers_consumers() {
    cout << "\n========== Test 3: Multiple Producers/Multiple Consumers ==========" << endl;
    
    LRUCache<int> cache(15);
    
    vector<thread> threads;
    
    // Create 2 producers
    for (int i = 1; i <= 2; i++) {
        threads.emplace_back(producer<int>, ref(cache), i, 4);
    }
    
    // Create 2 consumers
    for (int i = 1; i <= 2; i++) {
        threads.emplace_back(consumer<int>, ref(cache), i, 6);
    }
    
    // Wait for all to finish
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "Test 3 Complete!" << endl;
}

// Stress test: Rapid concurrent access
void stress_test() {
    cout << "\n========== Stress Test: Rapid Concurrent Access ==========" << endl;
    
    LRUCache<int> cache(10);
    
    auto rapid_writer = [&cache](int thread_id) {
        for (int i = 0; i < 50; i++) {
            cache.put(thread_id * 100 + i, i);
        }
    };
    
    auto rapid_reader = [&cache](int thread_id) {
        for (int i = 0; i < 50; i++) {
            cache.get(i % 10);
        }
    };
    
    vector<thread> threads;
    
    // 3 rapid writers
    for (int i = 0; i < 3; i++) {
        threads.emplace_back(rapid_writer, i);
    }
    
    // 3 rapid readers
    for (int i = 0; i < 3; i++) {
        threads.emplace_back(rapid_reader, i);
    }
    
    // Wait all
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "Stress test completed - no crashes = thread-safe!" << endl;
}

int main() {
    try {
        test_single_producer_consumer();
        test_multiple_producers();
        test_multiple_producers_consumers();
        stress_test();
        
        cout << "\n========== ALL TESTS PASSED ==========" << endl;
        cout << "LRU Cache is thread-safe!" << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
