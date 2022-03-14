#pragma once

#include "Texture3D.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"
#include "Shader.h"
#include "PrimitiveShape.h"

#include <FastNoise/FastNoiseLite.h>

#include <vector>

class WorleyGenerator
{
public:
	static void Initialize();
	static void CleanUp();

	static Texture2D* GenerateWorley2DFast(const glm::ivec2& dimensions, bool invert, float freq = 0.01f);
	static Texture2D* GenerateWorleyRGB2DFast(const glm::ivec2& dimensions, bool invert, float freq = 0.01f);
	static Texture3D* GenerateWorley3DFast(const glm::ivec3& dimensions, bool invert, float freq = 0.01f, FastNoiseLite::FractalType fractalType = FastNoiseLite::FractalType::FractalType_None, float gain = 0.4f);

	static Texture2D* GenerateWorley2D(unsigned int numPoints, const glm::ivec2& dimensions, bool invert);
	static Texture2D* GenerateWorleyRGB2D(unsigned int numPoints, const glm::ivec2& dimensions, bool invert);
	static Texture3D* GenerateWorley3D(unsigned int numPoints, const glm::ivec3& dimensions, bool invert, int tile, float persistence);

	static const std::string WORLEY_GENERATION_2D_SHADER_KEY;
	static const std::string WORLEY_GENERATION_3D_SHADER_KEY;

private:
	static std::vector<glm::vec2> GenerateWorleyData(int numPoints, const glm::ivec2& dimensions);

	static std::vector<glm::vec2> GeneratePoints2D(int numPoints, const glm::ivec2& dimensions);
	static std::vector<glm::vec3> GeneratePoints3D(int numPoints, const glm::ivec3& dimensions);

	static float ComputeFractalBounding(int octaves, float gain);

	static float ComputeWorleyPixel(const std::vector<glm::vec3>& points, const glm::vec3& pixelPos, int tile, int numPoints);

	static std::vector<glm::vec3>  GenPoints(int cellsPerAxis);

	static IFrameBuffer* frameBuffer;
	static Shader* shader2D;
	static Shader* shader3D;
};