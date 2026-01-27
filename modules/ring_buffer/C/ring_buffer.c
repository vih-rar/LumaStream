#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

ring_buffer_t* create_ring_buffer( size_t element_size, size_t size )
{
    if( size < 1 )
    {
        printf( "Size cannot be less than 1 \n" );
        return NULL;
    }
    ring_buffer_t* rb = malloc(sizeof( ring_buffer_t ) );
    
    if( !rb )
    {
        return NULL;
    }

    rb->buffer = malloc( element_size * size );

    if( !rb->buffer )
    {
        free( rb );
        return NULL;
    }

    rb->capacity = size;
    rb->count = 0;
    rb->writer = 0;
    rb->reader = 0;

    mtx_init( &rb->lock, mtx_plain );
    cnd_init( &rb->not_full );
    cnd_init( &rb->not_empty );

    return rb;
}

void destroy_ring_buffer( ring_buffer_t* rb )
{
    if( !rb )
    {
        return;
    }
    cnd_destroy( &rb->not_empty );
    cnd_destroy( &rb->not_full );
    mtx_destroy( &rb->lock );

    if( rb->buffer )
    {
        free(rb->buffer);
    }
    free( rb );
}

void* write_to_buffer( ring_buffer_t* rb, const void* dataptr )
{
    if( !rb || !dataptr )
    {
        return NULL;
    }
    mtx_lock(&rb->lock);
    void* ejected_ptr = NULL;

    if (rb->count == rb->capacity) {
        // Queue is full! We are about to overwrite the OLDEST pointer.
        // The oldest pointer is at the 'tail'.
        ejected_ptr = rb->buffer[rb->reader];
        
        // Move the tail forward because that old slot is being "erased"
        rb->reader = (rb->reader + 1) % rb->capacity;
        rb->count--;
    }

    // Now we can push the new pointer at the head
    rb->buffer[rb->writer] = dataptr;
    rb->writer = (rb->writer + 1) % rb->capacity;
    rb->count++;

    cnd_signal( &rb->not_empty );
    mtx_unlock( &rb->lock );
    return ejected_ptr;
}

void* get_stale_recycled(ring_buffer_t* rb, bool (*is_safe_callback)(void*)) {
    if (!rb || rb->count == 0) return NULL;

    mtx_lock(&rb->lock);
    void* ejected_ptr = NULL;

    // Search for a candidate that is NOT busy (STATE_READY in your enum)
    for (size_t i = 0; i < rb->count; i++) {
        size_t idx = (rb->reader + i) % rb->capacity;
        void* candidate = rb->buffer[idx];

        if (is_safe_callback(candidate)) {
            ejected_ptr = candidate;

            // Shift subsequent elements left to close the gap in the queue
            size_t current_idx = idx;
            for (size_t j = i; j < rb->count - 1; j++) {
                size_t next_idx = (current_idx + 1) % rb->capacity;
                rb->buffer[current_idx] = rb->buffer[next_idx];
                current_idx = next_idx;
            }

            rb->writer = (rb->writer == 0) ? rb->capacity - 1 : rb->writer - 1;
            rb->count--;
            break;
        }
    }

    mtx_unlock(&rb->lock);
    return ejected_ptr;
}

void read_from_buffer( ring_buffer_t* rb, void* data )
{
    if( !rb )
    {
        return;
    }

    mtx_lock( &rb->lock );
    if( rb->count == 0 )
    {
        // Non blocking
        mtx_unlock( &rb->lock );
        *(void**)data = NULL;
        return;
    }
    // while (rb->count == 0) {
    //     cnd_wait(&rb->not_empty, &rb->lock);
    // }

    void* ptr = rb->buffer[rb->reader];
    rb->reader = (rb->reader + 1) % rb->capacity;
    rb->count--;

    cnd_signal( &rb->not_full );
    mtx_unlock( &rb->lock );
    *(void**)data = ptr; 
}

bool is_buffer_empty( ring_buffer_t* rb )
{
    return buffer_space_available( rb ) == 0;
}

size_t buffer_space_available( ring_buffer_t* rb )
{
    mtx_lock( &rb->lock );
    size_t available = rb->count;
    mtx_unlock( &rb->lock );
    return available;
}

bool is_buffer_full( ring_buffer_t* rb )
{
    mtx_lock( &rb->lock );
    bool full = rb->count == rb->capacity;
    mtx_unlock( &rb->lock );
    return full;
}
