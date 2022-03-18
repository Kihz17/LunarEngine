#pragma once

#include <glm/glm.hpp>

template <typename T> class Grid2D
{
public:
	Grid2D(const glm::ivec2& size, const glm::ivec2& offset)
		: size(size),
		offset(offset)
	{
		data = new T* [size.x];
		for (unsigned int x = 0; x < size.x; x++)
		{
			data[x] = new T[size.y];
		}
	}

	~Grid2D()
	{
		for (unsigned int x = 0; x < size.x; x++)
		{
			delete[] data[x]; // Delete height
		}

		delete[] data; // Delete width
	}

	bool InBounds(const glm::ivec2& pos)
	{
		glm::ivec2 p = pos + offset;

		return (p.x >= 0 && p.x < size.x)
			&& (p.y >= 0 && p.y < size.y);
	}

	T& Get(const glm::ivec2& pos)
	{
		glm::ivec2 index = pos + offset;
		return data[index.x][index.y];
	}

	void Set(const glm::ivec2& pos, const T& value)
	{
		glm::ivec2 index = pos + offset;
		data[index.x][index.y] = value;
	}

	glm::ivec2 Size() const
	{
		return size;
	}

	glm::ivec2 Offset() const
	{
		return offset;
	}

private:
	T** data;
	glm::ivec2 size;
	glm::ivec2 offset;
};