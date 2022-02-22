#pragma once

#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Shader.h"
#include "CubeMap.h"
#include "PrimitiveShape.h"
#include "Camera.h"
#include "Window.h"
#include "Texture2D.h"

#include <glm/glm.hpp>
#include <vector>

class LightingPass
{
public:
	LightingPass(IFrameBuffer* gBuffer, IFrameBuffer* eBuffer, const WindowSpecs* windowSpecs, ITexture* shadowMaps, std::vector<float>& cascadeLevels);
	virtual ~LightingPass();

	void DoPass(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition);

private:
	IFrameBuffer* geometryBuffer;
	IFrameBuffer* environmentBuffer;

	Shader* shader;

	// Shadow Mapping
	ITexture* shadowMaps;
	std::vector<float>& cascadeLevels;

	PrimitiveShape quad;
};