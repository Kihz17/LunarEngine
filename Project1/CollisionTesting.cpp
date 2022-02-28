#include "CollisionTesting.h"

bool CollisionTesting::TestSphereSphere(const glm::vec3& centerA, const float radiusA, const glm::vec3& centerB, const float radiusB, const glm::vec3& velocityA, const glm::vec3& velocityB, float& t)
{
	glm::vec3 s = centerB - centerA; // Vector between center of the two spheres
	glm::vec3 relativeMotion = velocityB - velocityA; // Relative motion sphereB with respect to stationary sphereA
	float radiusSum = radiusA + radiusB;
	float c = glm::dot(s, s) - radiusSum * radiusSum;

	if (c < 0.0f) // Already intersecting, not moving towards a collision
	{
		t = 0.0f;
		return true;
	}

	float a = glm::dot(relativeMotion, relativeMotion); // Checks if we are moving away from eachother
	if (a < std::numeric_limits<float>::epsilon()) return false; // Spheres not moving relative to eachother

	float b = glm::dot(relativeMotion, s);
	if (b >= 0.0f) return false; // Spheres not moving towards eachother

	float d = (b * b) - (a * c);
	if (d < 0.0f) return false; // No real-valued root, spheres do not intersect

	t = (-b - sqrt(d)) / a;
	return true;
}

bool CollisionTesting::TestSpherePlane(const glm::vec3& a, const glm::vec3& b, float r, const glm::vec3& normal, float dotProduct)
{
	// Get the distance for A and B from the plane
	float distA = glm::dot(a, normal) - dotProduct;
	float distB = glm::dot(b, normal) - dotProduct;

	// Intersects if on different sides of the plane (distances have different signs)
	if (distA * distB < 0.0f) return true;

	// Intersects if start or end position within radius of the plane
	if (abs(distA) <= r || abs(distB) < r) return true;

	return false;
}