The following document explains the design considerations made for this project

*** Size of frame pool: ***

Buffer size - Height * Width * Bytes_per_pixel
Bytes_per_pixel depend on format as follows:
RGB: 3 bytes
YUV (Video): 1.5/2 bytes
HEIC (Apple): 3,4,6,8 bytes per pixel

I have considered HEIC standard 3 bytes per pixel, on full HD 1920*1080 frames
Therefore, buffer size = 1920 * 1080 * 3 = 6.2 MB

Alignment by 64 bytes can cause a maximum overhead of 63 bytes per alignment/buffer. 

With that in mind, the total memory footprint of each buffer is 6.2MB + 63 bytes =~ 6.2MB. 

The size of the buffer determines the size of our queues. 

The queue contains FrameBuffer_t structs:
typedef struct {
    uint32_t id;            // 4 bytes
    void* virt_addr;        // 16 bytes in 64 bit machine
    size_t size;            // 8 bytes in 64 bit machine
    uint64_t timestamp_ns;  // 8 bytes
    P_State state;          // 4 bytes
    uint32_t lens_id;       // 4 bytes
} FrameBuffer_t;            // 44 bytes

Queue size will be size of pool - 1, to ensure maximum sensing while 1 frame is being processed. There will be 2 queues of this size. 

Assuming pool of size 2 - 
Total memory = 2 * 6.2MB + 2 * 44B =~ 12.4MB, 2*63B 126 bytes waste

Assuming pool of size 4 - 
Total memory = 4 * 6.2MB + 3 * 44B =~ 18.6MB, 4*63B 252 bytes waste

Assuming pool of size 6 - 
Total memory = 6 * 6.2MB + 5 * 44B =~ 37.2MB, 6*63B 378 bytes waste

Assuming pool of size 10 - 
Total memory = 10 * 6.2MB + 9 * 44B =~ 62MB, 10*63B 630 bytes waste

The waste is 0.001% of buffer size, so the benefits of aligning memory outweigh the overhead.
As for size, the size of 6 is a reasonable middle ground, given it's 37.2MB size and extra frames in case processing is slow. 

I go one step further and allocate all the 37.2 MB and queue pointers on initialization, to avoid heap fragmentation.

*** Use of LRU Cache ***


typedef struct {
    uint32_t lens_id;            // 4 bytes     
    char lens_name[64];          // 64 bytes
    float distortion_k[6];       // 6 × 4 = 24 bytes (radial distortion coefficients)
    float vignette_params[4];    // 4 × 4 = 16 bytes
    float chromatic_aberration[3]; // 3 × 4 = 12 bytes
    uint32_t calibration_date;   // 4 bytes
} LensProfile_t;                 // 124 bytes/128 bytes with padding for 64 byte alignment

Cache memory = (Entry size + overhead) * number of entries

Overhead - 
typedef struct Node {
    uint32_t key;       // 4 bytes
    void* value;        // 16 bytes
    struct Node* prev;  // 16 bytes
    struct Node* next;  // 16 bytes
} Node;                 // 52 bytes

// Hash map entry for collision handling
typedef struct HashNode {
    uint32_t key;       // 4 bytes
    Node* node;         // 16 bytes
    struct HashNode* next;  // 16 bytes
} HashNode;             // 36 bytes

// LRU Cache structure
typedef struct {
    uint32_t capacity;  // 4 byets
    uint32_t size;      // 4 bytes
    Node* head;         // 16 bytes
    Node* tail;         // 16 bytes
    HashNode* hash_table[HASH_SIZE];    // 16 bytes
} lru_cache_t;          // 56 bytes

Total overhead = 144 bytes

Cache size = (124+144 = 268 bytes) * number of entries
With 10 entries, cache size = 2680 Bytes = 2.7KB.

For normal use case, using 3 lenses, 100% hit rate
With 10+ lenses, 90%+ hit rate

With a 3KB cache overhead, we are getting better hit rate. A useful comparison will be to use miss penalty in these calculations to understand trade off.
With a miss penalty of 10 ms, we lose 33% of our frame budget (33 ms, for 30 fps).
The overhead on the other hand is 3KB/37.2MB =~ 1% of total memory. 
