#include "WorleyGenerator.h"
#include "FrameBuffer.h"
#include "Grid3D.h"
#include "ShaderLibrary.h"
#include "Utils.h"
#include "TextureManager.h"

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

Texture2D* WorleyGenerator::GenerateWorley2DFast(const glm::ivec2& dimensions, bool invert, float freq)
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

Texture2D* WorleyGenerator::GenerateWorleyRGB2DFast(const glm::ivec2& dimensions, bool invert, float freq)
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

Texture3D* WorleyGenerator::GenerateWorley3DFast(const glm::ivec3& dimensions, bool invert, float freq, FastNoiseLite::FractalType fractalType, float gain)
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	noise.SetSeed(Utils::RandInt(0, 1000000));
	noise.SetFrequency(freq);
	noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
	noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean);
	noise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
	noise.SetCellularJitter(1.0f);
	noise.SetFractalType(fractalType);
	noise.SetFractalGain(gain);

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

Texture2D* WorleyGenerator::GenerateWorley2D(unsigned int numPoints, const glm::ivec2& dimensions, bool invert)
{
	std::vector<glm::vec2> points = GeneratePoints2D(numPoints, dimensions);

	int size = dimensions.x * dimensions.y * 4;
	float* data = new float[size];

	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			glm::vec2 pixelPos = glm::vec2(x, y) / glm::vec2(dimensions);

			float closestDist = std::numeric_limits<float>::max();
			for (const glm::vec2& point : points)
			{
				float dist = glm::distance2(pixelPos, point);
				if (dist < closestDist) closestDist = dist;
			}

			if (invert)
			{
				closestDist = 1.0f - closestDist;
			}

			data[i] = closestDist;
			data[i + 1] = closestDist;
			data[i + 2] = closestDist;
			data[i + 3] = 1.0f;
			i += 4;
		}
	}

	Texture2D* texture = TextureManager::CreateTexture2D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
	delete[] data;
	return texture;
}

Texture2D* WorleyGenerator::GenerateWorleyRGB2D(unsigned int numPoints, const glm::ivec2& dimensions, bool invert)
{
	std::vector<glm::vec2> points1 = GeneratePoints2D(numPoints, dimensions);
	std::vector<glm::vec2> points2 = GeneratePoints2D(numPoints, dimensions);
	std::vector<glm::vec2> points3 = GeneratePoints2D(numPoints, dimensions);

	int size = dimensions.x * dimensions.y * 4;
	float* data = new float[size];

	// Red channel
	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			glm::vec2 pixelPos = glm::vec2(x, y) / glm::vec2(dimensions);

			float closestDist = std::numeric_limits<float>::max();
			for (const glm::vec2& point : points1)
			{
				float dist = glm::distance2(pixelPos, point);
				if (dist < closestDist) closestDist = dist;
			}

			if (invert)
			{
				closestDist = 1.0f - closestDist;
			}

			data[i] = closestDist;
			data[i + 3] = 1.0f;
			i += 4;
		}
	}

	// Green channel
	i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			glm::vec2 pixelPos = glm::vec2(x, y) / glm::vec2(dimensions);

			float closestDist = std::numeric_limits<float>::max();
			for (const glm::vec2& point : points2)
			{
				float dist = glm::distance2(pixelPos, point);
				if (dist < closestDist) closestDist = dist;
			}

			if (invert)
			{
				closestDist = 1.0f - closestDist;
			}

			data[i + 1] = closestDist;
			i += 4;
		}
	}

	// Blue channel
	i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			glm::vec2 pixelPos = glm::vec2(x, y) / glm::vec2(dimensions);

			float closestDist = std::numeric_limits<float>::max();
			for (const glm::vec2& point : points2)
			{
				float dist = glm::distance2(pixelPos, point);
				if (dist < closestDist) closestDist = dist;
			}

			if (invert)
			{
				closestDist = 1.0f - closestDist;
			}

			data[i + 2] = closestDist;
			i += 4;
		}
	}


	Texture2D* texture = TextureManager::CreateTexture2D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
	delete[] data;
	return texture;
}

Texture3D* WorleyGenerator::GenerateWorley3D(unsigned int numPoints, const glm::ivec3& dimensions, bool invert, int tile, float persistence)
{
	std::vector<glm::vec3> points = GeneratePoints3D(numPoints, dimensions);

	int size = dimensions.x * dimensions.y * dimensions.z * 4;
	float* data = new float[size];

	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			for (int z = 0; z < dimensions.z; z++)
			{
				glm::vec3 pixelPos = glm::vec3(x, y, z) / glm::vec3(dimensions);

				float v = ComputeWorleyPixel(points, pixelPos, tile, points.size());

				/*float maxValue = 1.0f + persistence + (persistence * persistence);
				v /= maxValue;*/

				if (invert)
				{
					v = 1.0f - v;
				}

				data[i] = v;
				data[i + 1] = v;
				data[i + 2] = v;
				data[i + 3] = 1.0f;
				i += 4;
			}
		}
	}

	/*const float fractalBounding = ComputeFractalBounding(octaves, gain);

	int i = 0;
	for (int x = 0; x < dimensions.x; x++)
	{
		for (int y = 0; y < dimensions.y; y++)
		{
			for (int z = 0; z < dimensions.z; z++)
			{
				glm::vec3 pixelPos = (glm::vec3(x, y, z)) / glm::vec3(dimensions);

				float closestDist = std::numeric_limits<float>::max();
				for (const glm::vec3& point : points)
				{
					float dist = glm::distance(pixelPos, point);
					if (dist < closestDist) closestDist = dist;
				}

				if (invert)
				{
					closestDist = 1.0f - closestDist;
				}

				data[i] = closestDist;
				data[i + 1] = closestDist;
				data[i + 2] = closestDist;
				data[i + 3] = 1.0f;
				i += 4;
			}
		}
	}*/

	Texture3D* texture = TextureManager::CreateTexture3D(data, GL_RGBA16F, GL_RGBA, GL_FLOAT, dimensions.x, dimensions.y, dimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, false);
	delete[] data;
	return texture;
}

std::vector<glm::vec2> WorleyGenerator::GeneratePoints2D(int numPoints, const glm::ivec2& dimensions)
{
	std::vector<glm::vec2> points;
	points.resize(numPoints);

	for (int i = 0; i < numPoints; i++)
	{
		points[i] = glm::vec2(Utils::RandFloat(0.0f, 1.0f), Utils::RandFloat(0.0f, 1.0f));
	}

	return points;
}

std::vector<glm::vec3> WorleyGenerator::GeneratePoints3D(int numPoints, const glm::ivec3& dimensions)
{
	std::vector<glm::vec3> points;
	points.resize(numPoints);

	for (int i = 0; i < numPoints; i++)
	{
		points[i] = glm::vec3(Utils::RandFloat(0.0f, 1.0f), Utils::RandFloat(0.0f, 1.0f), Utils::RandFloat(0.0f, 1.0f));
	}

	return points;
}

float WorleyGenerator::ComputeFractalBounding(int octaves, float gain)
{
	float g = abs(gain);
	float amp = g;
	float ampFractal = 1.0f;
	for (int i = 1; i < octaves; i++)
	{
		ampFractal += amp;
		amp *= g;
	}
	return 1.0f / ampFractal;
}

float WorleyGenerator::ComputeWorleyPixel(const std::vector<glm::vec3>& points, const glm::vec3& pixelPos, int tile, int numPoints)
{
	constexpr glm::ivec3 offsets[] =
	{
		// centre
		glm::ivec3(0,0,0),

		// front face
		glm::ivec3(0,0,1),
		glm::ivec3(-1,1,1),
		glm::ivec3(-1,0,1),
		glm::ivec3(-1,-1,1),
		glm::ivec3(0,1,1),
		glm::ivec3(0,-1,1),
		glm::ivec3(1,1,1),
		glm::ivec3(1,0,1),
		glm::ivec3(1,-1,1),

		// back face
		glm::ivec3(0,0,-1),
		glm::ivec3(-1,1,-1),
		glm::ivec3(-1,0,-1),
		glm::ivec3(-1,-1,-1),
		glm::ivec3(0,1,-1),
		glm::ivec3(0,-1,-1),
		glm::ivec3(1,1,-1),
		glm::ivec3(1,0,-1),
		glm::ivec3(1,-1,-1),

		// ring around centre
		glm::ivec3(-1,1,0),
		glm::ivec3(-1,0,0),
		glm::ivec3(-1,-1,0),
		glm::ivec3(0,1,0),
		glm::ivec3(0,-1,0),
		glm::ivec3(1,1,0),
		glm::ivec3(1,0,0),
		glm::ivec3(1,-1,0)
	};

	float minDist = std::numeric_limits<float>::max();
	for (const glm::vec3& point : points)
	{
		for (int i = 0; i < 27; i++)
		{
			glm::vec3 offset = glm::vec3(offsets[i]);

			glm::vec3 offsetPoint = point + (point * offset);
			minDist = glm::min(minDist, glm::distance2(pixelPos, offsetPoint));
		}
	}
	
	return glm::sqrt(minDist);
}

std::vector<glm::vec3> WorleyGenerator::GenPoints(int cellsPerAxis)
{
	std::vector<glm::vec3> points;
	points.resize(cellsPerAxis * cellsPerAxis * cellsPerAxis);
	float cellSize = 1.0f / cellsPerAxis;

	for (int x = 0; x < cellsPerAxis; x++)
	{
		for (int y = 0; y < cellsPerAxis; y++)
		{
			for (int z = 0; z < cellsPerAxis; z++)
			{
				glm::vec3 randomOffset = glm::vec3(Utils::RandFloat(0.0f, 1.0f), Utils::RandFloat(0.0f, 1.0f), Utils::RandFloat(0.0f, 1.0f)) * cellSize;
				glm::vec3 cellCorner = glm::vec3 (x, y, z) * cellSize;

				int i = x + cellsPerAxis * (y + z * cellsPerAxis);
				points[i] = cellCorner + randomOffset;
			}
		}
	}

	return points;
}