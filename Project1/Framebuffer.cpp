#include "FrameBuffer.h"

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

	if (depthBuffer.texture) delete depthBuffer.texture;

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
	glDrawBuffer(ColorBufferConversion::ConvertTypeToGLType(type));
}

void FrameBuffer::SetColorBufferWriteAttachment(unsigned int attachment) const
{
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
}

void FrameBuffer::SetColorBufferRead(ColorBufferType type) const
{
	glReadBuffer(ColorBufferConversion::ConvertTypeToGLType(type));
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

void FrameBuffer::AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture->GetID(), 0);

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


void FrameBuffer::SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBuffer->GetID());
}

//void Framebuffer::AddColorBuffer(const std::string& name, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap)
//{
//	ColorBuffer buffer;
//	buffer.texture = new Texture(internalFormat, format, dataFormat, width, height, filter, wrap, false);
//
//	Bind();
//	buffer.attachment = colorBuffers.size();
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer.attachment, GL_TEXTURE_2D, buffer.texture->GetID(), 0);
//
//	colorBuffers.insert({ name, buffer });
//
//	// We've added a new color buffer, we have to refresh our attachments array
//	UpdateAttachmentArray();
//
//	glDrawBuffers(colorBuffers.size(), attachments); // Tell OpenGL about our new color attachments 
//
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//	{
//		std::cout << "Framebuffer not complete!" << std::endl;
//	}	
//
//	Unbind();
//}


void FrameBuffer::SetDepthBuffer(ITexture* texture, int level)
{
	Bind();
	depthBuffer.texture = texture;
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->GetID(), level);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}

	Unbind();
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