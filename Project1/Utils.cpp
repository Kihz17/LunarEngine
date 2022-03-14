#include "Utils.h"
#include "Texture2D.h"
#include "Texture3D.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <iostream>
#include <random>

static std::random_device random;
static std::mt19937 rng(random());

float Utils::RandFloat(float min, float max)
{
	std::uniform_real_distribution<float> distr(min, max);
	return distr(rng);
}

int Utils::RandInt(int min, int max)
{
	std::uniform_int_distribution<int> distr(min, max);
	return distr(rng);
}

bool Utils::RandBool()
{
	std::uniform_int_distribution<int> distr(0, 1);
	return distr(rng);
}

void Utils::SaveTextureAsBMP(const std::string& savePath, Texture2D* texture)
{
	unsigned int size = texture->GetWidth() * texture->GetHeight() * 4;
	uint8_t* data = new uint8_t[size];
	glGetTextureImage(texture->GetID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, size, data);
	stbi_write_bmp(savePath.c_str(), texture->GetWidth(), texture->GetHeight(), 4, data);
	delete[] data;
}

void Utils::SaveTexture3DAsBMP(const std::string& savePath, Texture3D* texture)
{
	unsigned int size = texture->GetWidth() * texture->GetHeight() * 4;
	uint8_t* data = new uint8_t[size];

	for (int z = 0; z < texture->GetDepth(); z++)
	{
		glGetTextureSubImage(texture->GetID(), 0, 0, 0, z, texture->GetWidth(), texture->GetHeight(), 1, GL_RGBA, GL_UNSIGNED_BYTE, size, data);
		stbi_write_bmp((savePath + std::to_string(z) + ".bmp").c_str(), texture->GetWidth(), texture->GetHeight(), 4, data);
	}

	delete[] data;
}