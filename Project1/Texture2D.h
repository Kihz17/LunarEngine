#pragma once

#include "ITexture.h"

#include <string>

class Texture2D : public ITexture
{
public:
	virtual GLuint GetID() const override { return ID; }
	virtual void Bind() const override;
	virtual void BindToSlot(uint32_t slot = 0) const override;
	virtual void Unbind() const override;

	virtual void TextureParameterFloat(GLenum pname, float value) override;
	virtual void TextureParameterInt(GLenum pname, int value) override;
	virtual void TextureParameterFloatArray(GLenum pname, const float* value) override;
	virtual void TextureParameterIntArray(GLenum pname, const int* value) override;

	virtual int GetWidth() const override { return width; }
	virtual int GetHeight() const override { return height; }

	virtual TextureType GetType() const { return TextureType::TEXTURE_2D; }

private:
	friend class TextureManager;

	Texture2D(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true, bool hdr = false); // 2D constructor
	Texture2D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true); // Empty 2D constructor

	void SetupHDR(float* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);
	void Setup(unsigned char* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);

	virtual ~Texture2D();

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};