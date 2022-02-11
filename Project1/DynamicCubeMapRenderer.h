#pragma once

#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "CubeMap.h"

#include <glm/glm.hpp>

#include <vector>

class DynamicCubeMapRenderer
{
public:
	DynamicCubeMapRenderer();
	~DynamicCubeMapRenderer();

	CubeMap* GenerateDynamicCubeMap(const glm::vec3& center, std::vector<RenderSubmission>& submissions);

private:
	IFrameBuffer* frameBuffer;
	CubeMap* cubeMap;
};