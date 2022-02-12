#pragma once

#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Window.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <string>

class GeometryPass 
{
public:
	GeometryPass(const WindowSpecs* windowSpecs);
	virtual ~GeometryPass();

	void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition);

	IFrameBuffer* GetGBuffer() { return geometryBuffer; }

	const std::string G_SHADER_KEY = "gShader";

private:
	IFrameBuffer* geometryBuffer;
	IRenderBuffer* geometryRenderBuffer;
	
	Shader* shader;

	const WindowSpecs* windowSpecs;
};