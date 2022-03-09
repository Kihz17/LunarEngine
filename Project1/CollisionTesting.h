#pragma once

#include <glm/glm.hpp>

class CollisionTesting
{
public:
	static bool TestSphereSphere(const glm::vec3& centerA, const float radiusA, const glm::vec3& centerB, const float radiusB, const glm::vec3& velocityA, const glm::vec3& velocityB, float& t);
	static bool TestSpherePlane(const glm::vec3& a, const glm::vec3& b, float r, const glm::vec3& normal, float dotProduct);
	static bool TestSphereAABB(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbCenter, const glm::vec3& aabbSize, float& dist2);
	static bool TestAABBPlane(const glm::vec3& aabbCenter, const glm::vec3& aabbSize, const glm::vec3& normal, float dotProduct, float& r, float& dist);
	static bool IntersectRayAABB(const glm::vec3& p, const glm::vec3& d, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tMin, glm::vec3& q);
};