#include "Texture.h"

#include <stb/stb_image.h>

#include <iostream>

Texture::Texture(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, bool flip, bool hdr)
{
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
	glBindTexture(GL_TEXTURE_2D, ID);

	if (!hdr)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisoFilterLevel); // Retreive the maximum level of anisotropy our GPU can support
		glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY, anisoFilterLevel); // Use the anisotropic level we just retreived 
	}

	if (hdr)
	{
		if (stbi_is_hdr(path.c_str()))
		{
			float* imageData = stbi_loadf(path.c_str(), &width, & height, &channels, 0);
			if (imageData)
			{
				SetupHDR(imageData, filter, wrap, genMipMaps);
			}
			else
			{
				std::cout << "[ERROR] Failed to load HDR texture from path: " << path << std::endl;
			}

			stbi_image_free(imageData);
		}
		else
		{
			std::cout << "[ERROR] Texture is not HDR! Path: " << path << std::endl;
		}
	}
	else // Load non HDR texture
	{
		unsigned char* imageData = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (imageData)
		{
			Setup(imageData, filter, wrap, genMipMaps);
		}
		else
		{
			std::cout << "[ERROR] Failed to load texture from path: " << path << std::endl;
		}

		stbi_image_free(imageData);
	}


	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
{

}

Texture::~Texture()
{
	glDeleteTextures(1, &ID);
}

void Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::BindToSlot(uint32_t slot) const
{
	Bind();
	glActiveTexture(GL_TEXTURE0 + slot);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetupHDR(float* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
{
	if (channels == 3)
	{
		internalFormat = GL_RGB32F;
		format = GL_RGB;
	}
	else if (channels == 4)
	{
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, data);

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
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Setup(unsigned char* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
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

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	// Filtering
	if (filter == TextureFilterType::Linear)
	{
		if (genMipMaps)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		if (genMipMaps)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		}
		else
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Wrapping
	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
}

//////////////////////
//	CUBE MAP
//////////////////////

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
	Bind();
	glActiveTexture(GL_TEXTURE0 + slot);
}

void CubeMap::Unbind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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