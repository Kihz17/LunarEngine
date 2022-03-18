#pragma once

#include "CubeMap.h"
#include "PrimitiveShape.h"
#include "Shader.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"

#include <string>

class EquirectangularToCubeMapConverter
{
public:
	static void Initialize();
	static void CleanUp();

	static void ConvertEquirectangularToCubeMap(Texture2D* envMapHDR, CubeMap* cubeMap, PrimitiveShape* cube, unsigned int nativeWidth, unsigned int nativeHeight);

	static const std::string CUBE_MAP_CONVERT_SHADER_KEY;

private:
	static Shader* conversionShader;
	static IFrameBuffer* cubeMapBuffer;
};