#pragma once

#include "Window.h"
#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "CubeMap.h"
#include "Shader.h"
#include "PrimitiveShape.h"
#include "Texture2D.h"

#include "IRenderBuffer.h"
#include <glm/glm.hpp>

#include <vector>

class DynamicCubeMapRenderer
{
public:
	DynamicCubeMapRenderer(const WindowSpecs* windowSpecs);
	~DynamicCubeMapRenderer();

	CubeMap* GenerateDynamicCubeMap(const glm::vec3& center, std::unordered_map<int, std::vector<RenderSubmission*>>& submissions, const float fov, const float far, const float near, const WindowSpecs* windowSpecs);

private:
	IFrameBuffer* frameBuffer;
	IFrameBuffer* cubeMapFrameBuffer;
	IRenderBuffer* depthBuffer;

	CubeMap* cubeMap;

	Shader* shader;
	Shader* conversionShader;

	PrimitiveShape quad;
};