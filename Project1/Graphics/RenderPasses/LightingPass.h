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
	LightingPass(const WindowSpecs* windowSpecs, ITexture* shadowMaps, std::vector<float>& cascadeLevels);
	virtual ~LightingPass();

	void DoPass(ITexture* positionBuffer, ITexture* albedoBuffer, ITexture* normalBuffer, ITexture* effectsBuffer, ITexture* environmentBuffer, ITexture* shadowSoftness,
		const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition, PrimitiveShape& quad);

private:
	Shader* shader;

	// Shadow Mapping
	ITexture* shadowMaps;
	std::vector<float>& cascadeLevels;
};