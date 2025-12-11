#pragma once
#include <atomic>
#include <memory>

namespace lunar 
{
	template<typename T, size_t kCapacityPow2>
	class JobQueue
	{
	public:
		// Local Only
		bool Push(const T& item)
		{
			int tail = mTail.load(std::memory_order_relaxed); // We are the only one who looks at tail, therefore we can be relaxed because we don't need other ordering
			int head = mHead.load(std::memory_order_acquire); // Acquire ensures stealers can see the correct tail and head locations

			if (tail - head == kCapacity) return false; // Buffer is full
			
			// No sync needed here because we are the only ones wo can write
			mBuffer[tail & (kCapacity - 1)] = item; 

			// Ensures writes before this barrier (buffer write) can be seen by stealers after they acquire the tail
			std::atomic_thread_fence(std::memory_order_release);

			mTail.store(tail + 1, std::memory_order_relaxed); // Relaxed because we are the only writer to the tail

			return true;
		}

		// Local Only
		bool Pop(T& out) 
		{
			int tail = mTail.load(std::memory_order_relaxed) - 1;
			mTail.store(tail, std::memory_order_relaxed);

			// Ensures stealer sees updated tail
			std::atomic_thread_fence(std::memory_order_seq_cst);

			int head = mHead.load(std::memory_order_relaxed); // relaxed because order is guaranteed by seq_cst
			if (head > tail) // Queue is empty
			{
				mTail.store(head, std::memory_order_relaxed); // Restore tail to head pos
				return false;
			}

			out = mBuffer[tail & (kCapacity - 1)];

			if (head != tail) return true; // Tail is not the same as head, no need to consider a race with stealers

			// Possible to be in a rase with stealers, CAS to resolve
			// Stealer increments head when stealing
			// Owner (us) also increments head when at the last element
			// Whoever increments head first wins the element
			int expected = head;
			if (!mHead.compare_exchange_strong(expected, // We expect the head value we see
				head + 1, // We want to increment head by 1 instead of decrement tail since we're the last element
				std::memory_order_seq_cst, // Ensure everyone sees the updated writes
				std::memory_order_relaxed))
			{
				// We lost, someone already stole it from us, restore tail to where the stealer put the head
				mTail.store(tail + 1, std::memory_order_relaxed);
				return false;
			}

			// We won!
			return true;
		}

		// Any Thread
		bool Steal(T& out)
		{
			int head = mHead.load(std::memory_order_acquire);

			// Keep trying to steal until:
			// A. queue is empty
			// B. We beat all other threads to steal the next item
			while (true)
			{
				int tail = mTail.load(std::memory_order_acquire);
				if (head >= tail)
				{ // Empty, nothing to steal
					return false;
				}

				out = mBuffer[head & (kCapacity - 1)];

				int expected = head;
				if (mHead.compare_exchange_strong(
					expected,
					head + 1,
					std::memory_order_seq_cst,
					std::memory_order_relaxed))
				{
					// Successfully claimed
					return true;
				}

				// Lost the CAS, this means another stealer took it already, just retry with the new head position
				head = expected;
			}
		}

		// Sync thread, only at frame barrier
		bool QueueBatch(const T* items, size_t count)
		{
			if (count == 0) return true;

			// IMPORTANT!!! We are assuming that anyone calling this function has paused the threads meaning that there 
			// are no active or future Push/Pop/Steal operations until this process is completed
			int head = mHead.load(std::memory_order_relaxed);
			int tail = mTail.load(std::memory_order_relaxed);

			// Check capacity
			if (tail - head + static_cast<int>(count) > static_cast<int>(kCapacity))
				return false; // Not enough room!

			int index = tail;
			for (size_t i = 0; i < count; ++i)
			{
				mBuffer[index & (kCapacity - 1)] = items[i];
				index++;
			}

			// Ensure buffer writes happen before the tail store. This means the next time the local threads access their queue's tail it will have the new buffer data 
			std::atomic_thread_fence(std::memory_order_release);

			mTail.store(tail + static_cast<int>(count), std::memory_order_relaxed);

			return true;
		}

	private:
		static constexpr size_t kCapacity = kCapacityPow2;

		std::atomic<int> mHead; // Front of queue, where we work steal from
		std::atomic<int> mTail; // End of queue, where we perform local pop and push

		T mBuffer[kCapacity]; // Ring buffer
	};
}
