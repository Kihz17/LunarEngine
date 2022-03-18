#pragma once

#include "vendor/imgui/imgui.h"

#include <glm/glm.hpp>

struct Component
{
	virtual ~Component() = default;

	virtual void ImGuiUpdate() = 0;

protected:
	Component() = default;
};