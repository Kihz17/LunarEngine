#pragma once

#include "IRenderBuffer.h"

class RenderBuffer : public IRenderBuffer
{
public:
	RenderBuffer(GLenum internalFormat, int width, int height);
	virtual ~RenderBuffer();

	virtual GLuint GetID() const override {	return ID; }

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void UpdateStorage(GLenum internalFormat, int width, int height) const override;
private:
	GLuint ID;
};