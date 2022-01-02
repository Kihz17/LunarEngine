#pragma once

#include "Texture.h"

#include <string>
#include <unordered_map>

struct ColorBuffer
{
	GLuint ID;
	unsigned int attachment;
};

class Framebuffer
{
public:
	Framebuffer();
	virtual ~Framebuffer();

	void Bind();
	void Unbind();

	void AddColorBuffer(const std::string& name, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap);
	void SetRenderBuffer(int width, int height, GLenum internalFormat, GLenum attachmentType);

	void BindColorBuffer(const std::string& name, uint32_t slot = 0);
private:
	void UpdateAttachmentArray();

	GLuint ID;
	GLuint renderBufferID;
	std::unordered_map<std::string, ColorBuffer> colorBuffers;
	unsigned int* attachments;
};