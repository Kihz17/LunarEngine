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

class Texture
{
public:
	Texture(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true, bool hdr = false); // 2D constructor
	Texture(TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true); // Empty 2D constructor
	virtual ~Texture();

	void Bind() const;
	void BindToSlot(uint32_t slot = 0) const;
	void Unbind() const; 

	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

private:
	void SetupHDR(float* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);
	void Setup(unsigned char* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps);

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};

class CubeMap
{
public:
	CubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true);
	CubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type = GL_UNSIGNED_BYTE);
	virtual ~CubeMap();

	void Bind() const;
	void BindToSlot(uint32_t slot = 0) const;
	void Unbind() const;

	void ComputeMipmap() const;

	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

private:
	void SetupCubeMap(unsigned char* data, int index, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, GLenum type = GL_UNSIGNED_BYTE);

	GLuint ID;
	int width, height, channels;
	float anisoFilterLevel; // Represents this texture's anisotropic filtering level
	GLenum internalFormat, format;
};