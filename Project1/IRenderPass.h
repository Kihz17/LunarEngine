#pragma once

#include "RenderSubmission.h"

#include <vector>

class IRenderPass
{
public:
	virtual ~IRenderPass() = default;

	virtual void DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view) = 0;
};