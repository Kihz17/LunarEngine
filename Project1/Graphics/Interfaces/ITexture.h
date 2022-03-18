#pragma once

#include "GLCommon.h"

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
	ClampToBorder,
	Repeat
};

enum class TextureType
{
	TEXTURE_2D,
	CUBE_MAP,
	TEXTURE_2D_ARRAY,
	TEXTURE_3D
};

class ITexture
{
public:
	virtual ~ITexture() = default;

	virtual GLuint GetID() const = 0;
	virtual void Bind() const = 0;
	virtual void BindToSlot(uint32_t slot = 0) const = 0;
	virtual void Unbind() const = 0;

	virtual void TextureParameterFloat(GLenum pname, float value) = 0;
	virtual void TextureParameterInt(GLenum pname, int value) = 0;
	virtual void TextureParameterFloatArray(GLenum pname, const float* value) = 0;
	virtual void TextureParameterIntArray(GLenum pname, const int* value) = 0;

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;

	virtual TextureType GetType() const = 0;
};