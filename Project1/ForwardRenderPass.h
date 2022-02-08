#pragma once

#include "IRenderPass.h"
#include "IFrameBuffer.h"
#include "Shader.h"
#include "Window.h"

class ForwardRenderPass : public IRenderPass
{
public:
	ForwardRenderPass(IFrameBuffer* geometryBuffer, const WindowSpecs* windowSpecs);
	virtual ~ForwardRenderPass();

	virtual void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view) override;

private:
	IFrameBuffer* geometryBuffer;
	Shader* shader;

	const WindowSpecs* windowSpecs;
};