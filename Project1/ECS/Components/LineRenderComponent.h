#pragma once

#include "Component.h"

#include <glm/glm.hpp>

struct LineRenderComponent : Component
{
	LineRenderComponent() = default;
	LineRenderComponent(const glm::vec3& p1, const glm::vec3& p2)
		: p1(p1),
		p2(p2)
	{}

	virtual ~LineRenderComponent() {}

	virtual void ImGuiUpdate() override
	{

	}

	glm::vec3 p1, p2;
};