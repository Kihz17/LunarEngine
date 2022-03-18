#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const void* data, unsigned int size, GLenum usage, int indexTarget)
	: ID(0)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBufferData(GL_UNIFORM_BUFFER, size, data, usage);

	if (indexTarget > -1)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, indexTarget, ID);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::UniformBuffer(unsigned int size, GLenum usage, int indexTarget)
	: ID(0)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);

	if (indexTarget > -1)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, indexTarget, ID);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(1, &ID);
}

void UniformBuffer::Bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

void UniformBuffer::Unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::BindToIndex(unsigned int index) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, index, ID);
}

void UniformBuffer::SubData(unsigned int offset, unsigned int size, const void* data) const
{
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}