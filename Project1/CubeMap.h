#pragma once

#include "ITexture.h"

#include <vector>
#include <string>

class CubeMap : public ITexture
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

	void ComputeMipmap() const;

	virtual int GetWidth() const override { return width; }
	virtual int GetHeight() const override { return height; }

	virtual TextureType GetType() const { return TextureType::CUBE_MAP; }

private:
	friend class TextureManager;

	CubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true);
	CubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type = GL_UNSIGNED_BYTE);
	virtual ~CubeMap();

	void SetupCubeMap(unsigned char* data, int index, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, GLenum type = GL_UNSIGNED_BYTE);

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};