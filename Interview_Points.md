Zero-Copy Memory Management: Explain that you avoid memcpy by passing pointers between threads. Emphasize that the pool of FrameBuffer_t structs and their virt_addr are pre-allocated during camera_init to prevent memory fragmentation and allocation latency during the high-speed capture loop.

Temporal Priority Over FIFO: Standard queues are strictly FIFO. You modified this so that if the ISP lags, the system "hunts" for the oldest stale frame in the queue that isn't actively being processed. This ensures the ISP always receives the latest possible frame (minimizing "glass-to-glass" latency) while maintaining 60fps sensor timing.

Atomic State Protection: You used __atomic built-ins to manage P_State (e.g., STATE_BUSY_PROCESSING). This ensures that even though a pointer is "ejected" from a queue, the Sensor cannot touch it if the ISP is still reading from it.

Smart Recycling (Search & Shift): Describe the get_stale_recycled algorithm. Explain how it iterates to find a non-busy buffer and performs a Shift-Left operation within the circular buffer to maintain the correct temporal order of the remaining frames.