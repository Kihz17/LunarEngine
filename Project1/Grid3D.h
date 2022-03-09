#pragma once

#include <glm/glm.hpp>

template <typename T> class Grid3D
{
public:
	Grid3D(const glm::ivec3& size, const glm::ivec3& offset)
		: size(size),
		offset(offset)
	{
		data = new T**[size.x];
		for (unsigned int x = 0; x < size.x; x++)
		{
			data[x] = new T*[size.y];
			for (unsigned int y = 0; y < size.y; y++)
			{
				data[x][y] = new T[size.z];
			}
		}
	}

	~Grid3D()
	{
		for (unsigned int x = 0; x < size.x; x++)
		{
			for (unsigned int y = 0; y < size.y; y++)
			{
				delete[] data[x][y]; // Delete depth
			}

			delete[] data[x]; // Delete height
		}
		delete[] data; // Delete width
	}

	bool InBounds(const glm::ivec3& pos)
	{
		glm::ivec3 p = pos + offset;

		return (p.x >= 0 && p.x < size.x)
			&& (p.y >= 0 && p.y < size.y)
			&& (p.z >= 0 && p.z < size.z);
	}

	T& Get(const glm::ivec3& pos)
	{
		glm::ivec3 index = pos + offset;
		return data[index.x][index.y][index.z];
	}

	void Set(const glm::ivec3& pos, const T& value)
	{
		glm::ivec3 index = pos + offset;
		data[index.x][index.y][index.z] = value;
	}

	glm::ivec3 Size() const
	{
		return size;
	}

	glm::ivec3 Offset() const
	{
		return offset;
	}

private:
	T*** data;
	glm::ivec3 size;
	glm::ivec3 offset;
};