#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <optional>

using namespace std;
template <typename T>
class RingBuffer
{
public:
	/*
	* @brief: Initializes buffer array of size 'size' 
	* @params: Size of buffer in bytes
	* @returns: None
	*/
	RingBuffer( size_t size, int max_timeout);

	/*
	* @brief: Resets the ring buffer to empty
	* @params: None
	* @returns: None
	*/ 
	void reset();

	/*
	* @brief: Shows next item from buffer
	* @params: None
	* @returns: Item of type T
	*/
	std::optional<T> peek();

	/*
	* @brief: Retrieves next item from buffer
	* @params: None
	* @returns: Item of type T
	*/
	std::optional<T> get();
	
	/*
	* @brief: Adds item into buffer, either writing or overwriting
	* @params: Item of type T
	* @returns: None
	*/
	void putUnconditional( T item );
	
	/*
	* @brief: Adds item into buffer, wait if full
	* @params: Item of type T
	* @returns: true if successful, false if not
	*/
	std::optional<bool> put( T item );

	/*
	* @brief: Checks if buffer is full
	* @params: None
	* @returns: Boolean to signify if buffer is full
	*/
	bool isFull() const;
	
	/*
	* @brief: Checks if buffer is empty
	* @params: None
	* @returns: Boolean to signify if buffer is empty
	*/
	bool isEmpty() const;

	/*
	* @brief: Check the current size of buffer
	* @params: None
	* @returns: Size of buffer in bytes
	*/
	size_t getSize() const;
	
	/*
	* @brief: Check the maximum capacity of buffer
	* @params: None
	* @returns: Maximum capacity of buffer in bytes
	*/
	size_t getCapacity() const noexcept 
	{
		return max_size;
	}

private:
	std::unique_ptr<T[]> buf;
	size_t max_size = 0;
	mutable std::mutex buf_mutex;
	bool full = false;
	size_t adder = 0;
	size_t remover = 0;
	std::condition_variable buffer_not_empty;
	std::condition_variable buffer_not_full;
	std::chrono::seconds timeout;
};

template <typename T>
RingBuffer<T>::RingBuffer(size_t size, int max_timeout) :
	buf(make_unique<T[]>(size)),
	max_size(size),
	adder(0),
	remover(0),
	full(false)
{
	if (size == 0)
	{
		throw std::invalid_argument("Buffer size cannot be zero");
	}
	timeout = std::chrono::seconds( max_timeout );
}

template <typename T>
void RingBuffer<T>::reset()
{
	lock_guard<mutex> lock(buf_mutex);
	adder = remover = 0;
	full = false;
	buffer_not_empty.notify_all();
}

template <typename T>
bool RingBuffer<T>::isEmpty() const
{
	lock_guard<mutex> lock(buf_mutex);
	return (adder == remover) && !full;
}

template <typename T>
bool RingBuffer<T>::isFull() const
{
	lock_guard<mutex> lock(buf_mutex);
	return full;
}

template <typename T>
size_t RingBuffer<T>::getSize() const
{
	lock_guard<mutex> lock(buf_mutex);
	if (full)
	{
		return max_size;
	}
	else
	{
		if (adder >= remover)
		{
			return adder - remover;
		}
		else
		{
			return adder - remover + max_size;
		}
	}
}

template <typename T>
std::optional<bool> RingBuffer<T>::put(T item)
{
	unique_lock<mutex> lock(buf_mutex);
	// waits buffer to be not empty for specified time before returning NULL
	if( !buffer_not_full.wait_for(lock, timeout, [this] { return !full; }) )
	{
		return std::nullopt;
	}

	buf[adder] = item;	// insert item

	adder = (adder + 1) % max_size;	// increment adder
	full = (adder == remover);
	// cout << "is full? " << full << " \n";
	buffer_not_empty.notify_one();
	return true;
}

template <typename T>
void RingBuffer<T>::putUnconditional(T item)
{
	lock_guard<mutex> lock1(buf_mutex);

	buf[adder] = item;	// insert item

	if (full)
	{
		remover = (remover + 1) % max_size;	// previous item got overwritten, move remover to next oldest item 
	}
	adder = (adder + 1) % max_size;	// increment adder
	full = (adder == remover);
	// cout << "is full? " << full << " \n";
	buffer_not_empty.notify_one();
}

template <typename T>
std::optional<T> RingBuffer<T>::get()
{
	unique_lock<mutex> lock(buf_mutex);
	// waits buffer to be not empty for specified time before returning NULL
	if( !buffer_not_empty.wait_for(lock, timeout, [this] { return !((adder == remover) && !full); }) )
	{
		return std::nullopt;
	}

	auto response = buf[remover];

	remover = (remover + 1) % max_size;
	full = false;
	buffer_not_full.notify_one();

	return response;
}

template <typename T>
std::optional<T> RingBuffer<T>::peek()
{
	unique_lock<mutex> lock(buf_mutex);
	if( !buffer_not_empty.wait_for(lock, timeout, [this] { return !((adder == remover) && !full); }) )
	{
		return std::nullopt;
	}

	return buf[ remover ];
}

