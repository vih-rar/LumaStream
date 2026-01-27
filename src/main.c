#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "aligned_malloc.h"
#include "ring_buffer.h"
#include "lru_cache.h"

// --- Constants & Configuration ---
#define FRAME_WIDTH       1920
#define FRAME_HEIGHT      1080
#define BUFFER_COUNT      4      // Typical for triple-buffering + 1 spare
#define ALIGNMENT         64     // Cache-line alignment for Apple Silicon
#define METADATA_CACHE_SZ 10     // Max lens profiles in LRU
volatile bool running = false;

typedef enum POOL_STATES
{
    STATE_BUSY_WRITING,
    STATE_BUSY_PROCESSING,
    STATE_READY
} P_State;

typedef struct {
    uint32_t lens_id;
    float gain_factor;
} LensProfile_t;

// --- Buffer Metadata ---
typedef struct {
    uint32_t id;
    void* virt_addr;        // Allocated via your aligned_malloc
    size_t size;
    uint64_t timestamp_ns;  // To simulate sync
    P_State state;       // enum for states
    uint32_t lens_id;
} FrameBuffer_t;

typedef struct {
    // 1. The Parking Lot (Storage)
    // We allocate 4 actual blocks of memory here.
    FrameBuffer_t* pool[BUFFER_COUNT];

    // 2. The Conveyor Belt (Flow)
    // This stores POINTERS to the buffers in the pool above.
    // It is initialized to hold 'FrameBuffer_t*' types.
    ring_buffer_t* ready_to_process_queue;
    ring_buffer_t* ready_to_write_queue;

    // 3. The Knowledge Base (Optimization)
    // Maps a 'LensID' to a 'CalibrationData' struct.
    lru_cache_t* lens_metadata_cache;
    uint32_t processed_count;
    mtx_t lock;
    uint32_t sensor_dropped_frames;
    uint32_t isp_dropped_frames;
} CameraDevice_t;

bool is_buffer_safe_to_overwrite(void* ptr) {
    FrameBuffer_t* buf = (FrameBuffer_t*)ptr;
    // Only recycle if it's waiting in the queue, NOT currently in the ISP
    return __atomic_load_n(&buf->state, __ATOMIC_ACQUIRE) == STATE_READY;
}

// Mock function simulating slow hardware access
LensProfile_t* load_lens_params_from_eeprom(uint32_t id) {
    usleep(20000); // 20ms "Hardware Latency"
    LensProfile_t* p = malloc(sizeof(LensProfile_t));
    p->lens_id = id;
    p->gain_factor = 1.2f + (id * 0.1f);
    return p;
}

// High-resolution timer for metadata
uint64_t get_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}


void simulate_sensor_capture(FrameBuffer_t* buf) {
    if (!buf || !buf->virt_addr) 
    {
        printf("[ERROR] Invalid buffer passed to simulate_sensor_capture\n");
        return;
    }
    // In a real Apple driver, this would be a DMA transfer.
    // We'll fill the buffer with a "gradient" that changes over time
    // so we can visually or programmatically verify frames are unique.
    uint8_t* data = (uint8_t*)buf->virt_addr;
    uint8_t frame_seed = (uint8_t)(buf->id % 255);

    if( buf->size != FRAME_HEIGHT*FRAME_WIDTH )
    {
        printf("[ERROR] Invalid buffer size passed to simulate_sensor_capture\n");
        return;
    }
    // We use a simple loop to simulate "writing" to memory
    // In a real-world scenario, you'd use a faster method or hardware.
    for (size_t i = 0; i < buf->size; i++) {
        data[i] = (uint8_t)((i + frame_seed) % 256);
    }

    // Attach metadata: Simulate a shifting Lens ID (e.g., zooming)
    buf->lens_id = (buf->id / 10) % 5; // Changes Lens ID every 10 frames
    buf->timestamp_ns = get_timestamp_ns(); 
}

void processing( FrameBuffer_t* buf, LensProfile_t* profile )
{
    if( !buf || !buf->virt_addr )
    {
        printf("[ERROR] Invalid buffer passed to processing\n");
        return;
    }
    uint8_t* data = (uint8_t*)buf->virt_addr;
    float gain = profile->gain_factor;

    // Just touch the first few pixels to simulate work without killing the CPU
    for (size_t i = 0; i < 100; i++) {
        data[i] = (uint8_t)(data[i] * gain > 255 ? 255 : data[i] * gain);
    }

    // Simulate "ISP Latency" - heavier processing takes longer
    usleep(1000000); // 5ms of "math"
}

// --- Module 1: Initialization ---

/**
 * TODO: Implement 'camera_init'
 * 1. Use your 'aligned_malloc' to allocate memory for each pool[i].virt_addr.
 * 2. Initialize your Ring Buffer to store FrameBuffer_t pointers.
 * 3. Initialize your LRU Cache to store simulated "Lens Correction Matrices".
 */

void camera_init(CameraDevice_t* dev)
{
    dev->ready_to_process_queue = create_ring_buffer( sizeof( void* ), BUFFER_COUNT );
    dev->ready_to_write_queue = create_ring_buffer( sizeof( void* ), BUFFER_COUNT );

    if( !dev->ready_to_process_queue || !dev->ready_to_write_queue )
    {
        printf("Queue creation failed \n");
        return;
    }

    size_t frame_bytes = FRAME_WIDTH * FRAME_HEIGHT; // Assuming 8-bit/YUV/etc.

    for(int i = 0; i < BUFFER_COUNT; i++) {
        // 1. Allocate the struct itself
        dev->pool[i] = malloc(sizeof(FrameBuffer_t));
        if( !dev->pool[i] )
        {
            return;
        }
        // 2. Allocate the actual large pixel memory (Zero-Copy area)
        dev->pool[i]->virt_addr = aligned_malloc(frame_bytes, ALIGNMENT);
        if( !is_aligned( dev->pool[i]->virt_addr , ALIGNMENT ))
        {
            printf( "Aligned malloc failed\n");
            return;
        }
        dev->pool[i]->size = frame_bytes;
        dev->pool[i]->id = i;
        dev->pool[i]->state = STATE_READY;

        // 3. Push the POINTER to the struct into the queue
        FrameBuffer_t* ptr = dev->pool[i];
        write_to_buffer(dev->ready_to_write_queue, ptr );
    }
    dev->lens_metadata_cache = lru_cache_create( BUFFER_COUNT );
    dev->isp_dropped_frames = 0;
    dev->sensor_dropped_frames = 0;
    dev->processed_count = 0;
    mtx_init( &dev->lock, mtx_plain );

}

void camera_deinit(CameraDevice_t* dev)
{
    if( !dev )
    {
        return;
    }
    mtx_destroy( &dev->lock );
    
    for( int i = 0; i<BUFFER_COUNT; i++ )
    {
        free_aligned( dev->pool[i]->virt_addr );
        free( dev->pool[i] );
    }

    if( dev->ready_to_process_queue )
    {
        destroy_ring_buffer( dev->ready_to_process_queue );
    }
    
    if( dev->ready_to_write_queue )
    {
        destroy_ring_buffer( dev->ready_to_write_queue );
    }

    if( dev->lens_metadata_cache )
    {
        lru_cache_free( dev->lens_metadata_cache );
    }
}

// --- Module 2: The Producer (Hardware/Sensor) ---

/**
 * TODO: Implement 'sensor_thread_loop'
 * Goal: Simulate the 60fps hardware interrupt.
 * 1. usleep(16666) to simulate frame arrival.
 * 2. Acquire a free buffer (you may need a second ring buffer for "Empty" frames).
 * 3. Simulate DMA: Write a pattern into the aligned memory.
 * 4. PUSH: Use your 'ring_buffer' to send the buffer pointer to the consumer.
 * 5. Handle "Full" state: If ring is full, drop the oldest to maintain real-time.
 */
int sensor_thread_loop( void* arg )
{
    CameraDevice_t* dev = (CameraDevice_t*)arg;
    while( running )
    {
        usleep( 500000 );
        FrameBuffer_t* buffer;
        read_from_buffer( dev->ready_to_write_queue, &buffer );
        
        if( !buffer )
        {
            buffer = get_stale_recycled( dev->ready_to_process_queue, is_buffer_safe_to_overwrite);
            if( buffer )
            {
                mtx_lock( &dev->lock );
                dev->isp_dropped_frames++;
                printf("[ISP] DROP! Recyled unprocessed buffer ID %d. Total ISP Drops: %u\n", buffer->id, dev->isp_dropped_frames);
                mtx_unlock( &dev->lock );
            }
        }

        if( buffer )
        {
            // printf("[SENSOR] Writing to Buffer ID: %u\n", buffer->id);
            __atomic_store_n(&buffer->state, STATE_BUSY_WRITING, __ATOMIC_RELEASE);

            simulate_sensor_capture( buffer );

            __atomic_store_n(&buffer->state, STATE_READY, __ATOMIC_RELEASE);
            printf("[SENSOR] Ready for processing Buffer ID: %u\n", buffer->id);

            FrameBuffer_t* recycled_buffer = write_to_buffer( dev->ready_to_process_queue, buffer );

            if( recycled_buffer )
            {
                printf("[SENSOR] Getting back unprocessed buffer ID: %u\n", recycled_buffer->id);
                write_to_buffer( dev->ready_to_write_queue, recycled_buffer );
            }
        }
        else
        {
            mtx_lock( &dev->lock );
            dev->sensor_dropped_frames++;
            printf("[SENSOR] DROP! No buffers available. Total Drops: %u\n", dev->sensor_dropped_frames);
            mtx_unlock( &dev->lock );
        }
    }
}

// --- Module 3: The Consumer (ISP/Image Processing) ---

/**
 * TODO: Implement 'isp_thread_loop'
 * Goal: Simulate the Neural Engine or ISP processing.
 * 1. POP: Block on your 'ring_buffer' until a frame is available.
 * 2. CACHE LOOKUP: Use your 'LRU_cache' to fetch lens calibration data.
 * - Logic: If profile for current "Lens ID" is in cache, use it.
 * - If not, simulate a slow I2C/Flash read and insert into LRU.
 * 3. PROCESS: Perform a dummy operation (e.g., calculate average brightness).
 * 4. RELEASE: Return the buffer to the "Empty" pool.
 */
int isp_thread_loop(void* arg)
{
    CameraDevice_t* dev = (CameraDevice_t*)arg;
    while( running )
    {
        FrameBuffer_t* buffer;
        read_from_buffer( dev->ready_to_process_queue, &buffer );

        if( buffer )
        {
            printf("[ISP] Processing Buffer ID: %u\n", buffer->id);
    
            __atomic_store_n(&buffer->state, STATE_BUSY_PROCESSING, __ATOMIC_RELEASE);

            mtx_lock( &dev->lock );
            LensProfile_t* p = lru_cache_get(dev->lens_metadata_cache, buffer->lens_id);
            if ( p == NULL ) 
            {
                // CACHE MISS: Simulate a slow I2C/EEPROM read from the lens hardware
                printf("[ISP] Cache Miss! Loading Lens %d calibration...\n", buffer->lens_id);
                p = load_lens_params_from_eeprom(buffer->lens_id); 
                lru_cache_put(dev->lens_metadata_cache, buffer->lens_id, p);
            }
            mtx_unlock(&dev->lock);

            processing( buffer, p );
            
            __atomic_store_n(&buffer->state, STATE_READY, __ATOMIC_RELEASE);
            printf("[ISP] Processed buffer ID: %u | Lens: %u | Timestamp: %lu\n", 
                        buffer->id, buffer->lens_id, buffer->timestamp_ns);
            write_to_buffer( dev->ready_to_write_queue, buffer );
            mtx_lock(&dev->lock);
            dev->processed_count++;
            if (dev->processed_count % 30 == 0) {
                printf("[ISP] Processed frame ID: %u | Lens: %u | Timestamp: %lu\n", 
                        buffer->id, buffer->lens_id, buffer->timestamp_ns);
            }
            mtx_unlock(&dev->lock);
            // printf("[ISP] Buffer ID: %u Ready for Reuse\n", buffer->id);
        }
    }
}

// --- Main: The Orchestrator ---

int main(void) {
    CameraDevice_t iphone_camera;
    // pthread_t sensor_tid, isp_tid;

    printf("[System] Initializing LumaStream Camera Driver...\n");
    camera_init(&iphone_camera);

    running = true;

    thrd_t sensorThread;
    thrd_t processingThread;

    thrd_create( &sensorThread, sensor_thread_loop, &iphone_camera );
    thrd_create( &processingThread, isp_thread_loop, &iphone_camera );

    // TODO: Launch Threads
    // pthread_create(&sensor_tid, NULL, sensor_thread_loop, &iphone_camera);
    // pthread_create(&isp_tid, NULL, isp_thread_loop, &iphone_camera);

    printf("[System] Pipeline running. Press Enter to stop...\n");
    fflush(stdin);
    getchar();

    running = false;
    thrd_join(sensorThread, NULL);
    thrd_join(processingThread, NULL);

    camera_deinit( &iphone_camera );
    
    // TODO: Clean up resources
    // 1. Join threads.
    // 2. Free aligned memory using your custom free.
    // 3. Destroy LRU and Ring Buffer.

    return 0;
}