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

void Framebuffer::AddColorBuffer(const std::string& name, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap)
{
	ColorBuffer buffer;
	glGenTextures(1, &buffer.ID);
	glBindTexture(GL_TEXTURE_2D, buffer.ID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataFormat, nullptr);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	buffer.attachment = colorBuffers.size();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer.attachment, GL_TEXTURE_2D, buffer.ID, 0);

	colorBuffers.insert({ name, buffer });

	// We've added a new color buffer, we have to refresh our attachments array
	UpdateAttachmentArray();

	glDrawBuffers(colorBuffers.size(), attachments); // Tell OpenGL about our new color attachments 
}

void Framebuffer::SetRenderBuffer(int width, int height, GLenum internalFormat, GLenum attachmentType)
{
	glGenRenderbuffers(1, &renderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBufferID);
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

void Framebuffer::BindColorBuffer(const std::string& name, uint32_t slot)
{
	if (colorBuffers.count(name) <= 0)
	{
		std::cout << "Could not find color buffer with name '" << name << "'!" << std::endl;
		return;
	}
		

	const ColorBuffer& buffer = colorBuffers.at(name);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, buffer.ID);
}