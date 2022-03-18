#include "RenderBuffer.h"

RenderBuffer::RenderBuffer(GLenum internalFormat, int width, int height)
	: ID(0)
{
	glGenRenderbuffers(1, &ID);
	glBindRenderbuffer(GL_RENDERBUFFER, ID);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer()
{
	glDeleteRenderbuffers(1, &ID);
}

void RenderBuffer::Bind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, ID);
}

void RenderBuffer::Unbind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderBuffer::UpdateStorage(GLenum internalFormat, int width, int height) const
{
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height); 
}