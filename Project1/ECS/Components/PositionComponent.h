#pragma once

#include "Component.h"

struct PositionComponent : public Component
{
	PositionComponent(glm::vec3 value) : value(value) {}
	PositionComponent() : value(glm::vec3(0.0f)) {}

	glm::vec3 value;
};