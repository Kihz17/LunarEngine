#pragma once

#include "vendor/imgui/imgui.h"

#include <glm/glm.hpp>

struct Component
{
	virtual ~Component() = default;

protected:
	Component() = default;
};