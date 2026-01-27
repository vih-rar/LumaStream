Aligned Malloc - Use Cases in One Sentence Each
Hardware Requirements

DMA Transfers - DMA controllers require buffer addresses at specific boundaries (typically 4 or 32 bytes) or they will fault/transfer incorrect data.
Ethernet Controllers - Network packet buffers must be aligned to prevent the MAC from corrupting frame data during transmission/reception.
USB Endpoints - USB controller descriptors and buffers need alignment to ensure proper data transfer without bus errors.
SD/MMC Cards - Block-oriented storage devices require sector buffers aligned for efficient DMA access during read/write operations.
Audio Codecs - I2S/SAI DMA buffers need alignment to prevent audio glitches and ensure smooth sample streaming.
SPI Flash - Page programming buffers must be aligned for optimal DMA transfer speeds during flash write operations.
Display Controllers - Frame buffers need alignment for DMA scanout to prevent tearing and ensure efficient pixel data fetching.
Camera Interfaces - Image capture buffers require alignment for the camera DMA to write incoming pixel data correctly.
CAN Bus - Message buffers in some CAN controllers need alignment for proper mailbox organization and DMA access.
ADC/DAC DMA - Sample buffers must be aligned for continuous conversion modes using DMA circular buffering.


Performance Optimization

SIMD/NEON Operations - Vector instructions require 16-byte aligned data or they will cause alignment faults or use slower unaligned load paths.
Cache Line Alignment - Prevents false sharing between CPU cores by ensuring each core's data occupies separate cache lines (typically 64 bytes).
Memory Bandwidth - Aligned accesses allow the memory controller to fetch data in single transactions instead of multiple partial reads.
Atomic Operations - Some atomic instructions require naturally aligned addresses to guarantee atomicity on certain architectures.
Prefetching - CPU prefetchers work more efficiently with aligned data structures, improving cache hit rates.


System Architecture

Page Tables - Memory management unit (MMU) page table entries must be aligned to page boundaries (4KB, 2MB, etc.).
Scatter-Gather DMA - Descriptor lists for scatter-gather DMA must be aligned for the DMA engine to parse them correctly.
Shared Memory IPC - Inter-process communication buffers need alignment to avoid cache coherency issues in multi-core systems.
RTOS Task Stacks - Some real-time operating systems require stack memory aligned to specific boundaries for context switching efficiency.
Memory Pools - Fixed-size memory pool allocators benefit from alignment to reduce fragmentation and improve allocation speed.


Protocol Requirements

Zero-Copy Networking - Network stacks require aligned buffers to pass packets directly to hardware without copying.
Cryptography Accelerators - Hardware crypto engines need aligned input/output buffers for AES, SHA, and other operations.
Video Codecs - Hardware video encoders/decoders require aligned frame buffers for efficient macroblock processing.
File Systems - Sector-aligned buffers are needed for direct I/O to storage devices bypassing the page cache.
GPU Transfers - Data sent to graphics processors must be aligned for efficient PCI Express or bus transfers.