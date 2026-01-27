#include <iostream>
#include <vector>
#include <cmath>
#include "ring_buffer.h"

using namespace std;

// Demo: Fast producer with overwriting
template <typename T>
void fastProducerUnconditional(RingBuffer<T>& buffer, vector<T> data) {
	for (auto item : data) {
		cout << "[Fast Producer] Putting: " << item << endl;
		buffer.putUnconditional(item);  // Never blocks, overwrites old data
		this_thread::sleep_for(chrono::milliseconds(50));
	}
	cout << "[Fast Producer] Finished producing " << data.size() << " items" << endl;
}

// Demo: Fast producer with no overwriting
template <typename T>
void fastProducer( RingBuffer<T>& buffer, vector<T> data )
{
	for(auto item : data)
	{
		cout << "[Fast Producer] Putting: " << item << endl;
		buffer.put( item );  // Never blocks, overwrites old data
		this_thread::sleep_for( chrono::milliseconds( 50 ) );
	}
	cout << "[Fast Producer] Finished producing " << data.size() << " items" << endl;
}

// Demo: Slow producer with overwriting
template <typename T>
void slowProducerUnconditional(RingBuffer<T>& buffer, vector<T> data) {
	for ( auto item : data ) {
		cout << "[Slow Producer] Putting: " << item << endl;
		buffer.putUnconditional(item);  // Never blocks, overwrites old data if buffer full
		this_thread::sleep_for(chrono::milliseconds(2000));  // Slow production
	}
	cout << "[Slow Producer] Finished producing " << data.size() << " items" << endl;
}

// Demo: Slow producer with no overwriting
template <typename T>
void slowProducer( RingBuffer<T>& buffer, vector<T> data )
{
	for(auto item : data)
	{
		cout << "[Slow Producer] Putting: " << item << endl;
		buffer.put( item );  // Never blocks, overwrites old data if buffer full
		this_thread::sleep_for( chrono::milliseconds( 2000 ) );  // Slow production
	}
	cout << "[Slow Producer] Finished producing " << data.size() << " items" << endl;
}

// Demo: Slow consumer
template <typename T>
void slowConsumer(RingBuffer<T>& buffer, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		auto value = buffer.get();  // Blocks when empty, waits for producer
		
		if( value.has_value() )
			cout << "[Slow Consumer] Got: " << *value << endl;
		else
			cout << "[Slow Consumer] Timed out!" << endl;
		this_thread::sleep_for(chrono::milliseconds(1100));  // Simulate slow processing
	}
	cout << "[Slow Consumer] Finished consuming " << size << " items" << endl;
}

// Demo: Fast consumer
template <typename T>
void fastConsumer(RingBuffer<T>& buffer, size_t size ) {
	for(size_t i = 0; i < size; ++i)
	{
		auto value = buffer.get();  // Blocks when empty, waits for producer
		
		if( value.has_value() )
			cout << "[Fast Consumer] Got: " << *value << endl;
		else
			cout << "[Fast Consumer] Timed out!" << endl;
		this_thread::sleep_for(chrono::milliseconds(50));  // Fast processing
	}
	cout << "[Fast Consumer] Finished consuming " << size << " items" << endl;
}


int main() {
	cout << "Testing message streaming with ring buffer" << endl;
	vector<string> message = { "The", "quick", "brown", "fox", "leapt", "Across", "the", "room" };
	size_t size = message.size() - 4;
	RingBuffer<string> messageBuffer( size, 2 );
	thread msgProducer( fastProducer< string >, ref(messageBuffer), message );
	this_thread::sleep_for(chrono::milliseconds(50));
	thread msgConsumer( slowConsumer< string >, ref(messageBuffer), message.size() );

	msgProducer.join();
	msgConsumer.join();

}