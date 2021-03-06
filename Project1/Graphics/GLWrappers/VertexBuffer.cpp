#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(float* vertices, uint32_t size)
	: size(size)
{
	glCreateBuffers(1, &this->ID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(uint32_t size)
	: size(size)
{
	glCreateBuffers(1, &this->ID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &this->ID);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, uint32_t size)
{
	if (size > this->size)
	{
		std::cout << "Cannot sub data because the given size was larger than what was allocated!" << std::endl;
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->ID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data); // Redefines the data in the VBO
}