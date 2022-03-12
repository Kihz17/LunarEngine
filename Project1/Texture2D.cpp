#include "Texture2D.h"

#include <stb/stb_image.h>

#include <iostream>

Texture2D::Texture2D(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, bool flip, bool hdr)
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
			float* imageData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
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

Texture2D::Texture2D(void* data, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
	: width(width),
	height(height)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataFormat, data);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::Texture2D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
	: width(width),
	height(height)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataFormat, nullptr);

	if (filter == TextureFilterType::Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filter == TextureFilterType::Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (wrap == TextureWrapType::ClampToEdge)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &ID);
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture2D::BindToSlot(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	Bind();
}

void Texture2D::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::TextureParameterFloat(GLenum pname, float value)
{
	glTextureParameterf(ID, pname, value);
}

void Texture2D::TextureParameterInt(GLenum pname, int value)
{
	glTextureParameteri(ID, pname, value);
}

void Texture2D::TextureParameterFloatArray(GLenum pname, const float* value)
{
	glTextureParameterfv(ID, pname, value);
}

void Texture2D::TextureParameterIntArray(GLenum pname, const int* value)
{
	glTextureParameteriv(ID, pname, value);
}

void Texture2D::SetupHDR(float* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
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
	else if (wrap == TextureWrapType::ClampToBorder)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else if (wrap == TextureWrapType::Repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::Setup(unsigned char* data, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
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