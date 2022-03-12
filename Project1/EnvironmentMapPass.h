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

	void DoPass(CubeMap* cubeMap, const glm::mat4& projection, const glm::mat4& view, PrimitiveShape& cube);

	IFrameBuffer* GetEnvironmentBuffer() { return environmentBuffer; }
	ITexture* GetEnvironmentTexture() { return envMapTexture; }

	static const std::string CUBE_MAP_DRAW_SHADER_KEY;

private:
	IFrameBuffer* environmentBuffer;
	Shader* shader;
	ITexture* envMapTexture;

	const WindowSpecs* windowSpecs;
};