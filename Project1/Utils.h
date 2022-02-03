#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Utils
{
public:
	static float RandFloat(float min, float max);

	static constexpr float PI() { return 3.14159265358979323846f; }
	static constexpr glm::vec3 FrontVec() { return glm::vec3(0.0f, 0.0f, 1.0f); }
	static constexpr glm::vec3 UpVec() { return glm::vec3(0.0f, 1.0f, 0.0f); }
	static constexpr glm::vec3 RightVec() { return glm::vec3(1.0f, 0.0f, 0.0f); }
};