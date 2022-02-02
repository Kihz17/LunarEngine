#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Utils
{
public:
	static float RandFloat(float min, float max);

	static constexpr float PI() { return 3.14159265358979323846f; }
};