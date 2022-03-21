#pragma once

#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "Shader.h"
#include "Window.h"
#include "SimpleFastVector.h"

class ForwardRenderPass
{
public:
	ForwardRenderPass(IFrameBuffer* geometryBuffer);
	virtual ~ForwardRenderPass();

	void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const WindowSpecs* windowSpecs);

private:
	IFrameBuffer* geometryBuffer;
	Shader* shader;
};