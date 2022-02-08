#include "TextureArray.h"

#include <stb/stb_image.h>

#include <iostream>

TextureArray::TextureArray(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, depth, 0, format, dataFormat, nullptr);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

TextureArray::~TextureArray()
{
	glDeleteTextures(1, &ID);
}

void TextureArray::Bind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}

void TextureArray::BindToSlot(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}

void TextureArray::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::TextureParameterFloat(GLenum pname, float value)
{
	glTextureParameterf(ID, pname, value);
}

void TextureArray::TextureParameterInt(GLenum pname, int value)
{
	glTextureParameteri(ID, pname, value);
}

void TextureArray::TextureParameterFloatArray(GLenum pname, const float* value)
{
	glTextureParameterfv(ID, pname, value);
}

void TextureArray::TextureParameterIntArray(GLenum pname, const int* value)
{
	glTextureParameteriv(ID, pname, value);
}