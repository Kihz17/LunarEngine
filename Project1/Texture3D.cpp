#include "Texture3D.h"

#include <stb/stb_image.h>

#include <iostream>

Texture3D::Texture3D(void* data, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
	: width(width),
	height(height),
	depth(depth)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_3D, ID);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, dataFormat, data);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_3D);

	glBindTexture(GL_TEXTURE_3D, 0);
}


Texture3D::Texture3D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
	: width(width),
	height(height),
	depth(depth)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_3D, ID);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, dataFormat, nullptr);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_3D);
}

Texture3D::~Texture3D()
{
	glDeleteTextures(1, &ID);
}

void Texture3D::Bind() const
{
	glBindTexture(GL_TEXTURE_3D, ID);
}

void Texture3D::BindToSlot(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	Bind();
}

void Texture3D::Unbind() const
{
	glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture3D::TextureParameterFloat(GLenum pname, float value)
{
	glTextureParameterf(ID, pname, value);
}

void Texture3D::TextureParameterInt(GLenum pname, int value)
{
	glTextureParameteri(ID, pname, value);
}

void Texture3D::TextureParameterFloatArray(GLenum pname, const float* value)
{
	glTextureParameterfv(ID, pname, value);
}

void Texture3D::TextureParameterIntArray(GLenum pname, const int* value)
{
	glTextureParameteriv(ID, pname, value);
}