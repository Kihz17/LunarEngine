#pragma once

#include "GLCommon.h"

class IUniformBuffer
{
public:
	virtual ~IUniformBuffer() = default;

	virtual GLuint GetID() const = 0;
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
	virtual void BindToIndex(unsigned int index) const = 0;
	virtual void SubData(unsigned int offset, unsigned int size, const void* data) const = 0;
};