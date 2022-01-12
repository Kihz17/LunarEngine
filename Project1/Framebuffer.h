#pragma once

#include "Texture.h"

#include <string>
#include <unordered_map>

struct ColorBuffer
{
	ITexture* texture = nullptr;
	unsigned int attachment;
};

class Framebuffer
{
public:
	Framebuffer();
	virtual ~Framebuffer();

	void Bind() const;
	void BindRead() const;
	void BindWrite() const;

	void Unbind() const;
	void UnbindRead() const;
	void UnbindWrite() const;

	//void AddCubeBuffer(const std::string& name, int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type);
	void AddColorBuffer(const std::string& name, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap);
	void SetRenderBuffer(int width, int height, GLenum internalFormat, GLenum attachmentType, bool attach = true);

	void BindRenderBuffer() const;
	void UpdateRenderBufferStorage(int width, int height, GLenum internalFormat) const;
	void BindColorBuffer(const std::string& name, uint32_t slot = 0);

	const ITexture* GetColorBufferTexture(const std::string& name) const;
private:
	void UpdateAttachmentArray();

	GLuint ID;
	GLuint renderBufferID;
	std::unordered_map<std::string, ColorBuffer> colorBuffers;
	unsigned int* attachments;
};