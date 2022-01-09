#pragma once

#include "GLCommon.h"

#include <string>
#include <vector>

enum class TextureFilterType
{
	None = 0,
	Linear,
	Nearest
};

enum class TextureWrapType
{
	None = 0, 
	ClampToEdge,
	Repeat
};

enum class TextureType
{
	TEXTURE_2D,
	CUBE_MAP
};

class ITexture
{
public:
	virtual ~ITexture() = default;

	virtual GLuint GetID() const = 0;
	virtual void Bind() const = 0;
	virtual void BindToSlot(uint32_t slot = 0) const = 0;
	virtual void Unbind() const = 0;

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;

	virtual TextureType GetType() const = 0;
};
class Texture : public ITexture
{
public:
	Texture(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true, bool hdr = false); // 2D constructor
	Texture(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true); // Empty 2D constructor
	virtual ~Texture();

	virtual GLuint GetID() const override { return ID; }
	virtual void Bind() const override;
	virtual void BindToSlot(uint32_t slot = 0) const override;
	virtual void Unbind() const override;

	virtual int GetWidth() const override { return width; }
	virtual int GetHeight() const override { return height; }

	virtual TextureType GetType() const { return TextureType::TEXTURE_2D; }

private:
	void SetupHDR(float* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);
	void Setup(unsigned char* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};

class CubeMap : public ITexture
{
public:
	CubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true);
	CubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type = GL_UNSIGNED_BYTE);
	virtual ~CubeMap();

	virtual GLuint GetID() const override { return ID; }
	virtual void Bind() const override;
	virtual void BindToSlot(uint32_t slot = 0) const override;
	virtual void Unbind() const override;

	void ComputeMipmap() const;

	virtual int GetWidth() const override { return width; }
	virtual int GetHeight() const override { return height; }

	virtual TextureType GetType() const { return TextureType::CUBE_MAP; }

private:
	void SetupCubeMap(unsigned char* data, int index, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, GLenum type = GL_UNSIGNED_BYTE);

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};