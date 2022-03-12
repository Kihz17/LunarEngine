#include "WorleyGenerator.h"
#include "FrameBuffer.h"
#include "Grid3D.h"
#include "ShaderLibrary.h"
#include "Utils.h"
#include "TextureManager.h"

#include <FastNoise/FastNoiseLite.h>
#include <vector>

IFrameBuffer* WorleyGenerator::frameBuffer = nullptr;
Shader* WorleyGenerator::shader2D = nullptr;
Shader* WorleyGenerator::shader3D = nullptr;

const std::string WorleyGenerator::WORLEY_GENERATION_2D_SHADER_KEY = "worleyGenShader2D";
const std::string WorleyGenerator::WORLEY_GENERATION_3D_SHADER_KEY = "worleyGenShader3D";

void WorleyGenerator::Initialize()
{
	frameBuffer = new FrameBuffer();
	shader2D = ShaderLibrary::Load(WORLEY_GENERATION_2D_SHADER_KEY, "assets/shaders/worleyTextureGenerator2D.glsl");
	shader2D->InitializeUniform("uResolution");
	shader2D->InitializeUniform("uNumWorleyPoints");
	for (int i = 0; i < 256; i++)
	{
		shader2D->InitializeUniform("uPointData[" + std::to_string(i) + "]");
	}

	shader3D = ShaderLibrary::Load(WORLEY_GENERATION_3D_SHADER_KEY, "assets/shaders/worleyTextureGenerator3D.glsl");
	shader3D->InitializeUniform("uResolution");
	shader3D->InitializeUniform("uNumWorleyPointsR");
	shader3D->InitializeUniform("uNumWorleyPointsG");
	shader3D->InitializeUniform("uNumWorleyPointsB");
	shader3D->InitializeUniform("uCurrentSlice");
	for (int i = 0; i < 256; i++)
	{
		shader3D->InitializeUniform("uPointDataR[" + std::to_string(i) + "]");
		shader3D->InitializeUniform("uPointDataG[" + std::to_string(i) + "]");
		shader3D->InitializeUniform("uPointDataB[" + std::to_string(i) + "]");
	}
}

void WorleyGenerator::CleanUp()
{
	delete frameBuffer;
}

Texture2D* WorleyGenerator::GenerateWorley2D(const glm::ivec2& dimensions, bool invert, float freq)
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	noise.SetSeed(Utils::RandInt(0, 1000000));
	noise.SetFrequency(freq);
	noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
	noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean);
	noise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
	noise.SetCellularJitter(1.0f);

	int size = dimensions.x * dimensions.y * 4;
	float* data = new float[size];

	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			float v = noise.GetNoise((float)x, (float)y);

			if (invert)
			{
				v *= -1.0f;
			}

			v = ((v + 1.0f) / 2.0f); // Convert from range [-1, 1] to [0, 1]

			data[i] = v;
			data[i + 1] = v;
			data[i + 2] = v;
			data[i + 3] = 1.0f;
			i += 4;
		}
	}

	Texture2D* texture = TextureManager::CreateTexture2D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
	delete[] data;
	return texture;
}

Texture2D* WorleyGenerator::GenerateWorleyRGB2D(const glm::ivec2& dimensions, bool invert, float freq)
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	noise.SetSeed(Utils::RandInt(0, 1000000));
	noise.SetFrequency(freq);
	noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
	noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean);
	noise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
	noise.SetCellularJitter(1.0f);

	int size = dimensions.x * dimensions.y * 4;
	float* data = new float[size];

	// Do red channel
	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			float v = noise.GetNoise((float)x, (float)y);

			if (invert)
			{
				v *= -1.0f;
			}

			v = ((v + 1.0f) / 2.0f); // Convert from range [-1, 1] to [0, 1]

			data[i] = v;
			data[i + 3] = 1.0f;
			i += 4;
		}
	}

	// Do green channel
	noise.SetSeed(Utils::RandInt(0, 1000000));
	i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			float v = noise.GetNoise((float)x, (float)y);

			if (invert)
			{
				v *= -1.0f;
			}

			v = ((v + 1.0f) / 2.0f); // Convert from range [-1, 1] to [0, 1]

			data[i + 1] = v;
			i += 4;
		}
	}

	// Do blue channel
	noise.SetSeed(Utils::RandInt(0, 1000000));
	i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			float v = noise.GetNoise((float)x, (float)y);

			if (invert)
			{
				v *= -1.0f;
			}

			v = ((v + 1.0f) / 2.0f); // Convert from range [-1, 1] to [0, 1]

			data[i + 2] = v;
			i += 4;
		}
	}

	Texture2D* texture = TextureManager::CreateTexture2D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
	delete[] data;
	return texture;
}

Texture3D* WorleyGenerator::GenerateWorley3D(const glm::ivec3& dimensions, bool invert, float freq)
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	noise.SetSeed(Utils::RandInt(0, 1000000));
	noise.SetFrequency(freq);
	noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
	noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean);
	noise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
	noise.SetCellularJitter(1.0f);

	int size = dimensions.x * dimensions.y * dimensions.z * 4;
	float* data = new float[size];

	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			for (int z = 0; z < dimensions.z; z++)
			{
				float v = noise.GetNoise((float)x, (float)y, (float)z);

				if (invert)
				{
					v *= -1.0f;
				}

				v = ((v + 1.0f) / 2.0f); // Convert from range [-1, 1] to [0, 1]

				v = Utils::RandFloat(0.0f, 1.0f);

				data[i] = v;
				data[i + 1] = v;
				data[i + 2] = v;
				data[i + 3] = 1.0f;
				i += 4;
			}
		}
	}

	Texture3D* texture = TextureManager::CreateTexture3D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, dimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, false);
	delete[] data;
	return texture;
}