#pragma once

#include <iostream>

template <typename T, typename P, typename F> class PriorityQueue
{
private:
	struct Node
	{
		Node() = default;
		Node(const T& t, const P& p)
			: data(t), priority(p)
		{
		}

		T data;
		P priority;
	};

public:
	PriorityQueue()
		: data(nullptr),
		size(0),
		capacity(DEFAULT_SIZE)
	{
		data = new Node[capacity];
	}

	void Enqueue(const T& v, const P& prio)
	{
		if (size >= capacity)
		{
			Reserve(capacity * 2);
		}

		Node newNode(v, prio);
		if (compareFunction(data[0].priority, newNode.priority)) // Higher priority than our first element
		{
			// Add to the front, push everytihng down
			for (unsigned int i = size; i > 0; i--)
			{
				data[i] = data[i - 1];
			}

			data[0] = newNode;
			size++;
		}
		else // Add to the back
		{
			data[size++] = newNode;
		}

	/*	std::cout << "Adding to queue (new size: " << size <<")\n";
		for (int i = 0; i < size; i++)
		{
			std::cout << data[i].data << "\n";
		}*/
	}

	void Dequeue()
	{
		// Find next highest priority
		int nextBestPrio = -1;
		Node* bestNode = nullptr;
		for (unsigned int i = 1; i < size; i++)
		{
			if (!bestNode || compareFunction(data[i].priority, bestNode->priority))
			{
				nextBestPrio = i;
				bestNode = &data[i];
			}
		}

		data[0] = data[nextBestPrio];
		size--;

		// Move everything left after nextBestPrio index
		for (unsigned int i = nextBestPrio; i < size; i++)
		{
			data[i] = data[i + 1];
		}

	/*	std::cout << "Removing from queue (new size: " << size << ")\n";
		for (int i = 0; i < size; i++)
		{
			std::cout << data[i].data << "\n";
		}*/
	}

	T& Front()
	{
		return data[0].data;
	}

	void Clear()
	{
		delete[] data;
		size = 0;
		data = new Node[capacity];
	}

	bool TryGetPriority(const T& v, P& prio)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			if (v == data[i].data)
			{
				prio = data[i].priority;
				return true;
			}
		}

		return false;
	}

	void Update(const T& v, const P& newPrio)
	{
		int swapIndex = -1;
		for (unsigned int i = 0; i < size; i++)
		{
			if (data[i].data == v)
			{
				swapIndex = i;
				break;
			}
		}

		if (swapIndex != -1)
		{
			if (compareFunction(data[0].priority, newPrio)) // New prio is better, swap them
			{
				Node oldPrio = data[0];
				data[0] = data[swapIndex];
				data[swapIndex] = oldPrio;
			}
			else // Not better than front, update priority
			{
				data[swapIndex].priority = newPrio;
			}
		}

	/*	std::cout << "Updating queue (new size: " << size << ")\n";
		for (int i = 0; i < size; i++)
		{
			std::cout << data[i].data << "\n";
		}*/
	}

	unsigned int Size()
	{
		return size;
	}

	bool Empty()
	{
		return size <= 0;
	}

private:
	void Reserve(unsigned int value)
	{
		if (data == nullptr)
		{
			size = 0;
			capacity = 0;
		}

		Node* newArr = new Node[value];

		// Copy data into new resized array
		int oldSize = value < size ? value : size;
		for (int i = 0; i < oldSize; i++)
		{
			newArr[i] = data[i];
		}

		capacity = value;
		delete[] data;
		data = newArr;
	}

	Node* data;
	unsigned int size;
	unsigned int capacity;

	F compareFunction;

	static const unsigned int DEFAULT_SIZE = 10;
};