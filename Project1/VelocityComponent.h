#pragma once

#include "Component.h"

struct VelocityComponent : public Component
{
	VelocityComponent() : value(glm::vec3(0.0f)) {}

	virtual void ImGuiUpdate() override
	{

	}

	glm::vec3 value;
};