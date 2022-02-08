#pragma once

#include "GLCommon.h"

class IRenderBuffer
{
public:
	virtual ~IRenderBuffer() = default;

	virtual GLuint GetID() const = 0;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UpdateStorage(GLenum internalFormat, int width, int height) const = 0;
};