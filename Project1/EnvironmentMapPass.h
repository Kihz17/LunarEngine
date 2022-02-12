#pragma once

#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Window.h"
#include "Shader.h"
#include "CubeMap.h"
#include "Texture2D.h"
#include "PrimitiveShape.h"

#include <glm\glm.hpp>

class EnvironmentMapPass
{
public:
	EnvironmentMapPass(const WindowSpecs* windowSpecs);
	virtual ~EnvironmentMapPass();

	void DoPass(const glm::mat4& projection, const glm::mat4& view);

	IFrameBuffer* GetEnvironmentBuffer() { return environmentBuffer; }

	void SetEnvironmentMapEquirectangular(const std::string& path);

	CubeMap* GetCubeMap() { return envMapCube; }

	const std::string CUBE_MAP_DRAW_SHADER_KEY = "drawEnvShader";
	const std::string CUBE_MAP_CONVERT_SHADER_KEY = "hdrToCubeShader";

private:
	IFrameBuffer* environmentBuffer;
	Shader* shader;
	PrimitiveShape cube;

	Texture2D* envMapHDR;
	CubeMap* envMapCube;

	IFrameBuffer* cubeMapBuffer;
	Shader* conversionShader;

	const WindowSpecs* windowSpecs;
};