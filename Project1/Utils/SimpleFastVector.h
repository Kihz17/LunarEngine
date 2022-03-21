#pragma once

#include <iostream>

template <typename T> class SimpleFastVector
{
public:
	SimpleFastVector(int defaultCapactiy)
		: size(0),
		capacity(defaultCapactiy)
	{
		data = new T[capacity];
	}

	~SimpleFastVector()
	{
		delete[] data;
	}

	void push_back(T& value)
	{
		if (size >= capacity) reserve(capacity * 2);

		data[size++] = value;

		for (int i = 0; i < size; i++)
		{
			std::cout << "V: " << data[i] << "\n";
		}
		std::cout << "\n";
	}

	void clear()
	{
		size = 0;
	}

	void reserve(int size)
	{
		T* newData = new T[size];
		capacity = size;

		for (int i = 0; i < size; i++)
		{
			newData[i] = data[i];
		}

		delete[] data;
		data = newData;
	}

	T& operator[](int index)
	{
		return data[index];
	}

	T* begin() 
	{ 
		if (size == 0) return nullptr;
		return &data[0]; 
	}

	T* end()
	{
		if (size == 0) return nullptr;
		return &data[size - 1];
	}

private:
	T* data;
	int size, capacity;
};