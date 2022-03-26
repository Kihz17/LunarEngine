#pragma once

#include "Component.h"

struct ScaleComponent : public Component
{
	ScaleComponent(glm::vec3 value) : value(value) {}
	ScaleComponent() : value(glm::vec3(1.0f)) {}

	glm::vec3 value;
};