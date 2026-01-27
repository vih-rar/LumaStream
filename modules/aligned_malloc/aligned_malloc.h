#ifndef ALIGNED_MALLOC_H
#define ALIGNED_MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void* aligned_malloc( size_t size, size_t alignment );
void free_aligned(void* ptr);
int is_aligned(void* ptr, size_t alignment);

#endif