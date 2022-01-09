#include "Framebuffer.h"

#include <iostream>

Framebuffer::Framebuffer()
	: attachments(nullptr),
	renderBufferID(0)
{
	glGenFramebuffers(1, &ID);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &ID);

	std::unordered_map<std::string, ColorBuffer>::iterator it = colorBuffers.begin();
	while (it != colorBuffers.end()) // Destroy create textures
	{
		if (it->second.texture) delete it->second.texture;
		it++;
	}

	colorBuffers.clear();

	if (renderBufferID)
	{
		glDeleteRenderbuffers(1, &renderBufferID);
	}

	delete[] attachments;
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void Framebuffer::AddCubeBuffer(const std::string& name, int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type)
//{
//	ColorBuffer buffer;
//	buffer.texture = new CubeMap(width, minFilter, format, internalFormat, type);
//
//	Bind();
//	buffer.attachment = colorBuffers.size();
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer.attachment, , buffer.texture->GetID(), 0);
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

void Framebuffer::AddColorBuffer(const std::string& name, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap)
{
	ColorBuffer buffer;
	buffer.texture = new Texture(internalFormat, format, dataFormat, width, height, filter, wrap, false);

	Bind();
	buffer.attachment = colorBuffers.size();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer.attachment, GL_TEXTURE_2D, buffer.texture->GetID(), 0);

	colorBuffers.insert({ name, buffer });

	// We've added a new color buffer, we have to refresh our attachments array
	UpdateAttachmentArray();

	glDrawBuffers(colorBuffers.size(), attachments); // Tell OpenGL about our new color attachments 

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}	

	Unbind();
}

void Framebuffer::SetRenderBuffer(int width, int height, GLenum internalFormat, GLenum attachmentType, bool attach)
{
	Bind();
	glGenRenderbuffers(1, &renderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	if(attach)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBufferID);
	Unbind();
}

void Framebuffer::UpdateAttachmentArray()
{
	if (attachments)
		delete[] attachments;

	std::vector<ColorBuffer*> sortedBuffers;
	std::unordered_map<std::string, ColorBuffer>::iterator it = colorBuffers.begin();
	while (it != colorBuffers.end())
	{
		sortedBuffers.push_back(&it->second);
		it++;
	}

	// Sort the buffers
	for (int i = 0; i < sortedBuffers.size(); i++)
	{
		for (int j = i + 1; j < sortedBuffers.size(); j++)
		{
			if (sortedBuffers[j]->attachment < sortedBuffers[i]->attachment)
			{
				ColorBuffer* temp = sortedBuffers[i];
				sortedBuffers[i] = sortedBuffers[j];
				sortedBuffers[j] = temp;
			}
		}
	}

	int size = sortedBuffers.size();
	attachments = new unsigned int[size];
	for (int i = 0; i < size; i++)
	{
		attachments[i] = GL_COLOR_ATTACHMENT0 + sortedBuffers[i]->attachment;
	}
}

void Framebuffer::UpdateRenderBufferStorage(int width, int height, GLenum internalFormat) const
{
	BindRenderBuffer();
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

void Framebuffer::BindRenderBuffer() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
}

void Framebuffer::BindColorBuffer(const std::string& name, uint32_t slot)
{
	if (colorBuffers.count(name) <= 0)
	{
		std::cout << "Could not find color buffer with name '" << name << "'!" << std::endl;
		return;
	}
		
	const ColorBuffer& buffer = colorBuffers.at(name);
	buffer.texture->BindToSlot(slot);
}

const ITexture* Framebuffer::GetColorBufferTexture(const std::string& name) const
{
	if (colorBuffers.count(name) <= 0)
	{
		std::cout << "Could not find color buffer with name '" << name << "'!" << std::endl;
		return nullptr;
	}

	return colorBuffers.at(name).texture;
}