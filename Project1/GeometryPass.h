#pragma once

#include "IRenderPass.h"
#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Window.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <string>

class GeometryPass : public IRenderPass
{
public:
	GeometryPass(const WindowSpecs* windowSpecs);
	virtual ~GeometryPass();

	virtual void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view) override;

	IFrameBuffer* GetGBuffer() { return geometryBuffer; }

	const std::string G_SHADER_KEY = "gShader";

private:
	IFrameBuffer* geometryBuffer;
	IRenderBuffer* geometryRenderBuffer;
	
	Shader* shader;

	const WindowSpecs* windowSpecs;
};