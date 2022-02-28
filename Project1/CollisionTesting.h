#pragma once

#include <glm/glm.hpp>

class CollisionTesting
{
public:
	static bool TestSphereSphere(const glm::vec3& centerA, const float radiusA, const glm::vec3& centerB, const float radiusB, const glm::vec3& velocityA, const glm::vec3& velocityB, float& t);
	static bool TestSpherePlane(const glm::vec3& a, const glm::vec3& b, float r, const glm::vec3& normal, float dotProduct);
};