#pragma once

#include "ITexture.h"

class TextureArray : public ITexture
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

	virtual TextureType GetType() const { return TextureType::TEXTURE_2D_ARRAY; }

private:
	friend class TextureManager;

	TextureArray(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);
	virtual ~TextureArray();

	GLuint ID;
	int width, height, channels;
};