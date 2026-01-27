// test.c
#include "ring_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <threads.h>

int producer(void* arg) {
    ring_buffer_t* rb = (ring_buffer_t*)arg;
    
    for (int i = 0; i < 100; i++) {
        uint8_t data = i;
        write_to_buffer(rb, &data, 1);
        printf("Produced: %d\n", data);
        usleep(10000);  // 10ms - slow producer
    }
    return 1;
}

int consumer(void* arg) {
    ring_buffer_t* rb = (ring_buffer_t*)arg;
    
    for (int i = 0; i < 100; i++) {
        uint8_t data;
        read_from_buffer(rb, &data, 1);
        printf("Consumed: %d\n", data);
        usleep(50000);  // 50ms - slow consumer
    }
    return 1;
}

int main() {
    ring_buffer_t* rb = create_ring_buffer( sizeof( uint8_t), 10);
    
    thrd_t prodThread;
    thrd_t custThread;

    thrd_create( &prodThread, producer, rb );
    thrd_create( &custThread, consumer, rb );

    thrd_join( prodThread, NULL );
    thrd_join( custThread, NULL );

    destroy_ring_buffer( rb );
    return 0;
}