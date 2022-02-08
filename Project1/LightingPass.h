#pragma once

#include "IRenderPass.h"
#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Shader.h"
#include "CubeMap.h"
#include "PrimitiveShape.h"
#include "Camera.h"
#include "Window.h"
#include "Texture2D.h"

#include <glm/glm.hpp>

class LightingPass : public IRenderPass
{
public:
	LightingPass(IFrameBuffer* gBuffer, IFrameBuffer* eBuffer, const WindowSpecs* windowSpecs, glm::vec3& cameraPosition);
	virtual ~LightingPass();

	virtual void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view) override;

private:
	IFrameBuffer* geometryBuffer;
	IFrameBuffer* environmentBuffer;

	Shader* shader;

	glm::vec3& cameraPosition;

	PrimitiveShape quad;
};