#include "CubeMap.h"

#include <stb/stb_image.h>

#include <iostream>

CubeMap::CubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, bool flip)
{
	if (paths.size() != 6)
	{
		std::cout << "[ERROR] Cube map had " << paths.size() << " images instead of 6!" << std::endl;
		return;
	}

	if (flip)
	{
		stbi_set_flip_vertically_on_load(true);
	}
	else
	{
		stbi_set_flip_vertically_on_load(false);
	}

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (int i = 0; i < 6; i++)
	{
		unsigned char* imageData = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);
		if (imageData)
		{
			SetupCubeMap(imageData, i, filter, wrap, genMipMaps);
		}
		else
		{
			std::cout << "[ERROR] Failed to load cube map at index " << i << "! Path: " << paths[i] << std::endl;
		}

		stbi_image_free(imageData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeMap::CubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type)
	:
	width(width),
	height(width),
	format(format),
	internalFormat(internalFormat)
{
	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, type, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &ID);
}

void CubeMap::Bind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void CubeMap::BindToSlot(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	Bind();
}

void CubeMap::Unbind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeMap::TextureParameterFloat(GLenum pname, float value)
{
	glTextureParameterf(ID, pname, value);
}

void CubeMap::TextureParameterInt(GLenum pname, int value)
{
	glTextureParameteri(ID, pname, value);
}

void CubeMap::TextureParameterFloatArray(GLenum pname, const float* value)
{
	glTextureParameterfv(ID, pname, value);
}

void CubeMap::TextureParameterIntArray(GLenum pname, const int* value)
{
	glTextureParameteriv(ID, pname, value);
}

void CubeMap::SetupCubeMap(unsigned char* data, int index, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, GLenum type)
{
	if (channels == 1)
	{
		format = GL_RED;
	}
	else if (channels == 3)
	{
		format = GL_RGB;
	}
	else if (channels == 4)
	{
		format = GL_RGBA;
	}

	internalFormat = format;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, internalFormat, width, height, 0, format, type, data);

	// Filtering
	if (filter == TextureFilterType::Linear)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Wrapping
	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void CubeMap::ComputeMipmap() const
{
	Bind();
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
