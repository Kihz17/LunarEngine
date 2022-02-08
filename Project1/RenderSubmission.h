#pragma once

#include "RenderComponent.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct RenderSubmission
{
	RenderComponent* renderComponent = nullptr;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};
