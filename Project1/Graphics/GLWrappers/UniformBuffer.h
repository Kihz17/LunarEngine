#pragma once

#include "IUniformBuffer.h"

class UniformBuffer : public IUniformBuffer
{
public:
	UniformBuffer(const void* data, unsigned int size, GLenum usage, int indexTarget = -1);
	UniformBuffer(unsigned int size, GLenum usage, int indexTarget = -1);
	virtual ~UniformBuffer();

	virtual GLuint GetID() const override { return ID; }
	virtual void Bind() const override;
	virtual void Unbind() const override;
	virtual void BindToIndex(unsigned int index) const override;
	virtual void SubData(unsigned int offset, unsigned int size, const void* data) const override;

private:
	GLuint ID;
};