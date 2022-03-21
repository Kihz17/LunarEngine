#include "WaterPass.h"
#include "TextureManager.h"
#include "Utils.h"

#include <random>
#include <iostream>

#include <stb/stb_image_write.h>

static const float ONE_OVER_SQRT_2 = 0.7071067f;
static const float TWO_PI = 6.283185f;

float WaterPass::Phillips(const glm::vec2& pos, const glm::vec2& windDir, float windSpeed, float amplitude, float gravityAccel)
{
	float largestWave = (windSpeed * windSpeed) / gravityAccel; // Compute largest possible wave for wind speed
	float suppressedValue = largestWave / 1000.0f; // Supress waves smaller than this

	float posDotDir = glm::dot(pos, windDir);
	float pos2 = glm::dot(pos, pos);

	float phillips = amplitude * (expf(-1.0f / (pos2 * largestWave * largestWave))) / (pos2 * pos2 * pos2) * (posDotDir * posDotDir);

	if (posDotDir < 0.0f)
	{
		phillips *= 0.07f; // Wave is moving against the wind direction
	}

	return phillips * expf(-pos2 * suppressedValue * suppressedValue);
}

Texture2D* WaterPass::GenH0()
{
	glm::vec2* h0Data = new glm::vec2[(512 + 1) * (512 + 1)];
	float* wData = new float[(512 + 1) * (512 + 1)];

	int start = 512 / 2;
	float patchSize = 20.0f;

	glm::vec2 windDir = glm::normalize(glm::vec2(-0.4f, -0.9f));
	//glm::vec2 thing;
	//float v = 1.0f / sqrtf(windDir.x * windDir.x + windDir.y * windDir.y);
	//thing.x = windDir.x * v;
	//thing.y = windDir.y * v;

	float windSpeed = 6.5f;
	float amplitude = 0.45f * 1e-3f;
	float gravityAccel = -9.81f;
	glm::vec2 pixel;

	std::mt19937 gen;
	std::normal_distribution<> gaussian(0.0, 1.0);

	//int index = 0;
	for (int i = 0; i <= 512; i++)
	{
		pixel.y = (TWO_PI * (start - i)) / patchSize;

		for (int j = 0; j <= 512; j++)
		{
			pixel.x = (TWO_PI * (start - j)) / patchSize;

			int index = i * (512 + 1) + j;
			float sqrtPhillips = 0.0f;

			if (pixel.x != 0.0f || pixel.y != 0.0f)
			{
				sqrtPhillips = sqrtf(Phillips(pixel, windDir, windSpeed, amplitude, gravityAccel));
			}

			h0Data[index].x = (float)(sqrtPhillips * gaussian(gen) * ONE_OVER_SQRT_2);
			h0Data[index].y = (float)(sqrtPhillips * gaussian(gen) * ONE_OVER_SQRT_2);
		}
	}

	Texture2D* t = TextureManager::CreateTexture2D(h0Data, GL_RG32F, GL_RG, GL_FLOAT, 512 + 1, 512 + 1, TextureFilterType::Linear, TextureWrapType::Repeat);
	return t;
}

Texture2D* WaterPass::GenW()
{
	return nullptr;
}