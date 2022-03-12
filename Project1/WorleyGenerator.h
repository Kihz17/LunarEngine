#pragma once

#include "Texture3D.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"
#include "Shader.h"
#include "PrimitiveShape.h"

class WorleyGenerator
{
public:
	static void Initialize();
	static void CleanUp();

	static Texture2D* GenerateWorley2D(const glm::ivec2& dimensions, bool invert, float freq = 0.01f);
	static Texture2D* GenerateWorleyRGB2D(const glm::ivec2& dimensions, bool invert, float freq = 0.01f);
	static Texture3D* GenerateWorley3D(const glm::ivec3& dimensions, bool invert, float freq = 0.01f);

	static const std::string WORLEY_GENERATION_2D_SHADER_KEY;
	static const std::string WORLEY_GENERATION_3D_SHADER_KEY;
private:
	static IFrameBuffer* frameBuffer;
	static Shader* shader2D;
	static Shader* shader3D;
};