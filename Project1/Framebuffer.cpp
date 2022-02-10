#include "FrameBuffer.h"
#include "Texture2D.h"

#include <iostream>

FrameBuffer::FrameBuffer()
	: attachments(nullptr),
	renderBufferID(0)
{
	glGenFramebuffers(1, &ID);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &ID);

	if(attachments) delete[] attachments;
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void FrameBuffer::BindRead() const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
}

void FrameBuffer::BindWrite() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ID);
}

void FrameBuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::UnbindRead() const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void FrameBuffer::UnbindWrite() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::SetColorBufferWrite(ColorBufferType type) const
{
	glDrawBuffer(FrameBufferConversion::ConvertTypeToGLType(type));
}

void FrameBuffer::SetColorBufferWriteAttachment(unsigned int attachment) const
{
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
}

void FrameBuffer::SetColorBufferRead(ColorBufferType type) const
{
	glReadBuffer(FrameBufferConversion::ConvertTypeToGLType(type));
}

void FrameBuffer::SetColorBufferReadAttachment(unsigned int attachment) const
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
}

bool FrameBuffer::CheckComplete() const
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

ITexture* FrameBuffer::GetColorAttachment(const std::string& name)
{
	for (ColorBuffer& colorBuffer : colorBuffers)
	{
		if (colorBuffer.name == name) return colorBuffer.texture;
	}

	return nullptr;
}

void FrameBuffer::AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index, FrameBufferOperationType operationType)
{
	glFramebufferTexture2D(FrameBufferConversion::ConvertOperationToGL(operationType), GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture->GetID(), 0);

	if (index + 1 > colorBuffers.size())
	{
		colorBuffers.push_back({ name, texture });
	}
	else
	{
		colorBuffers[index] = { name, texture };
	}

	UpdateAttachmentArray(); // We've added a new color buffer, we have to refresh our attachments array

	glDrawBuffers(colorBuffers.size(), attachments); // Tell OpenGL about our new color attachments 
}

void FrameBuffer::SetDepthAttachment(ITexture* texture, FrameBufferOperationType operationType, int level)
{
	glFramebufferTexture(FrameBufferConversion::ConvertOperationToGL(operationType), GL_DEPTH_ATTACHMENT, texture->GetID(), level);
	depthBuffer = texture;
}

void FrameBuffer::SetDepthAttachment2D(Texture2D* texture, FrameBufferOperationType operationType, int level)
{
	glFramebufferTexture(FrameBufferConversion::ConvertOperationToGL(operationType), GL_DEPTH_ATTACHMENT, texture->GetID(), level);
	depthBuffer = texture;
}

void FrameBuffer::SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBuffer->GetID());
}

void FrameBuffer::UpdateAttachmentArray()
{
	if (attachments)
		delete[] attachments;

	int size = colorBuffers.size();
	attachments = new unsigned int[size];
	for (int i = 0; i < size; i++)
	{
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
}