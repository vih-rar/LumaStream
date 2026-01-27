cmake_minimum_required(VERSION 3.10)
project(LumaStream C)

# Use C11 for atomics and better thread support
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Threading library (pthreads)
find_package(Threads REQUIRED)

# --- Define Sub-Modules as Static Libraries ---

# Aligned Allocator
add_library(aligned_malloc modules/aligned_malloc/aligned_malloc.c)
target_include_directories(aligned_malloc PUBLIC modules/aligned_malloc/)

# Ring Buffer
add_library(ring_buffer modules/ring_buffer/C/ring_buffer.c)
target_include_directories(ring_buffer PUBLIC modules/ring_buffer/C/)

# LRU Cache
add_library(lru_cache modules/lru_cache/C/lru_cache.c)
target_include_directories(lru_cache PUBLIC modules/lru_cache/C/)

# --- Main Application ---

add_executable(LumaStream src/main.c)

# Include the 'include' folder for global camera types
target_include_directories(LumaStream PRIVATE include)

# Link everything together
# This tells CMake to handle all the header paths and binary linking
target_link_libraries(LumaStream 
    PRIVATE 
    aligned_malloc 
    ring_buffer 
    lru_cache 
    Threads::Threads
)