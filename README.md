The CameraStreamBuffering project simulates a camera sensor processing pipeline in a memory constrained system. The pipeline consists of a simulation of recording pixel data (1920*1080) and a simulation of processing it with some latency. 

The idea is to continue recording sensor data at the cost of unprocessed frames, to ensure processing of most latest frames rather than most sequential. A design choice can be made between blocking sensor and replacing unprocessed data, though both will lead to similar results. 

When processing is not delayed, this project uses effective memory management and a zero copy method by using a pool of predefined memory segments to hold pixel data. It takes the help of aligned malloc to ensure minimal cache replacements when accessing the large pools of data. Pointers to these pools reside in exclusive processing and sensing ring buffers. An LRU Cache is used to store lens configuration data for minimal processing times. 

By containing only pointers to data pools, the ring buffers avoid copying of data into memory segments and keeping the buffers of minimal pointer sizes. 

The sensor and processing happens on separate threads. To ensure exclusive writing and processing of data pools, we use enums to introduce pool states. While the 2 different ring buffers already handle exclusivity, this is a handy debugging tool to analyze last pool state and realize bottlenecks when meeting frame drops. 

All the memory is assigned at initialization, and freed in the destructor.

Debugging was done using GDB and Valgrind.

This was an attempt at a multithreaded frame buffer implementation, utilizing custom implementations of ring buffers, aligned mallocs, and LRU Cache.
