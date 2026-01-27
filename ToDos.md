Thread Wake-up on Exit: In your main, when you set running = false, the ISP thread might still be stuck inside read_from_buffer. A production fix involves "poisoning" the queue—pushing a special NULL pointer—to force the ISP thread to wake up, check the running flag, and exit gracefully.

Pool Slack (N+1): Currently, you have 4 buffers and a queue capacity of 4. In professional drivers, we often use a pool of 6 for a queue of 4. This extra "slack" allows the sensor to keep moving for two extra frames before it ever has to resort to "stealing" or recycling a frame from the ISP queue.

Formalize the Stats: You already have sensor_dropped_frames and isp_dropped_frames. Moving these into a dedicated Stats_t struct with a single mtx_lock for the whole block makes reporting much cleaner.

Logging Wit: Your logs are great for debugging. For production, you could wrap them in a DEBUG_LOG() macro that disappears in "Release" builds to save CPU cycles.