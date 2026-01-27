#include "aligned_malloc.h"
#include <stdio.h>

void* aligned_malloc( size_t size, size_t alignment )
{
    // check if power of 2
    if( ( alignment == 0 ) || ( alignment & ( alignment - 1 )) != 0 )
    {
        return NULL;
    }

    // alloc total space
    size_t total_size = size + alignment + sizeof( void* );
    void* raw_ptr = malloc( total_size );
    if( !raw_ptr )
    {
        return NULL;
    }
    // calculate alignment
    uintptr_t raw_addr = (uintptr_t)raw_ptr + sizeof( void* );  // round up to nearest size allocation to accomodate for original ptr
    uintptr_t aligned_addr = ( raw_addr + alignment - 1 ) & ~( alignment - 1 ); // create alignment and remove lsb

    // store original ptr at alignment - size(void*)
    void** stored_ptr = (void**)(aligned_addr - sizeof( void* )); // get address of space right before aligned address
    *stored_ptr = raw_ptr; // raw_ptr (original malloc of the aligned pointer) points to that address 
    // return aligned ptr
    return (void*)aligned_addr;
}
void free_aligned( void* ptr )
{
    if( ptr )
    {
        void** stored_ptr = (void**)(( uintptr_t )ptr - sizeof( void* )); // pointer to address of original pointer to aligned address
        free( *stored_ptr ); // dereference pointer to address 
    }
}
int is_aligned(void* ptr, size_t alignment)
{
    // Check for NULL pointer or invalid alignment
    if( !ptr || alignment == 0 )
    {
        return 0;
    }
    return( (uintptr_t)ptr & ( alignment - 1 ) ) == 0; 
}
/*
int main() {
    size_t size = 64;
    size_t alignment = 16;

    // Allocate aligned memory
    void* ptr = aligned_malloc(size, alignment);
    if (!ptr) {
        printf("Failed to allocate aligned memory.\n");
        return 1;
    }

    // Check alignment
    if (is_aligned(ptr, alignment)) {
        printf("Memory is aligned to %zu bytes.\n", alignment);
    } else {
        printf("Memory is NOT aligned to %zu bytes.\n", alignment);
    }

    // Free aligned memory
    free_aligned(ptr);
    printf("Aligned memory freed successfully.\n");

    return 0;
}
    */