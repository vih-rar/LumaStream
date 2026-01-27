#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <threads.h>  

// buffer class contains
typedef struct
{
    void** buffer;
    size_t writer;
    size_t reader;
    size_t capacity;
    size_t count;

    mtx_t lock;
    cnd_t not_empty;
    cnd_t not_full;

} ring_buffer_t;

ring_buffer_t* create_ring_buffer( size_t element_size, size_t size );
// create

void destroy_ring_buffer( ring_buffer_t* rb );
// destroy

void* write_to_buffer( ring_buffer_t* rb, const void* dataptr );
// write_to

void read_from_buffer( ring_buffer_t* rb, void* data );
// read_from

size_t buffer_space_available( ring_buffer_t* rb );
// available

bool is_buffer_empty( ring_buffer_t* rb );
// is_empty

bool is_buffer_full( ring_buffer_t* rb );
// is_full

void* get_stale_recycled(ring_buffer_t* rb, bool (*is_safe_callback)(void*));

#endif
