#pragma once

#include "RenderSubmission.h"
#include "Window.h"
#include "IFrameBuffer.h"
#include "Shader.h"
#include "SimpleFastVector.h"

class LinePass
{
public:
	LinePass();
	virtual ~LinePass();

	void DoPass(std::vector<LineRenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const WindowSpecs* windowSpecs);

private:
	Shader* shader;
};