#pragma once

#include <glm/glm.hpp>

class AABB
{
public:
	AABB(const glm::vec3& min, const glm::vec3 max);
	virtual ~AABB();


	glm::vec3 min;
	glm::vec3 max;
};