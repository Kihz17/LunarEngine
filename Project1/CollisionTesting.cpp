#include "CollisionTesting.h"

#include <iostream>

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

bool CollisionTesting::TestSphereAABB(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbCenter, const glm::vec3& aabbSize, float& dist2)
{
	glm::vec3 min = aabbCenter - aabbSize;
	glm::vec3 max = aabbCenter + aabbSize;

	dist2 = 0.0f;
	for (unsigned int i = 0; i < 3; i++)
	{
		float v = sphereCenter[i];
		// for each axis, count the excess distance outside of the AABB
		if (v < min[i]) dist2 += (min[i] - v) * (min[i] - v);
		if (v > max[i]) dist2 += (v - max[i]) * (v - max[i]);
	}

	return dist2 <= sphereRadius * sphereRadius;
}

bool CollisionTesting::TestAABBPlane(const glm::vec3& aabbCenter, const glm::vec3& aabbSize, const glm::vec3& normal, float dotProduct, float& r, float& dist)
{
	// Compute the projection interval radius of the bounding box onto L(t) = aabbCenter + t * normal
	r = aabbSize.x * glm::abs(normal.x) + aabbSize.y * glm::abs(normal.y) + aabbSize.z * glm::abs(normal.z);
	dist = glm::dot(normal, aabbCenter) - dotProduct; // Compute distance of aabb center from plane

	return glm::abs(dist) <= r;
}

bool CollisionTesting::IntersectRayAABB(const glm::vec3 & origin, const glm::vec3& direction, const glm::vec3 & aabbMin, const glm::vec3 & aabbMax, float& tMin, glm::vec3 & q)
{
	tMin = 0.0f;
	float tMax = std::numeric_limits<float>::max();

	for (unsigned int i = 0; i < 3; i++)
	{
		if (glm::abs(direction[i]) < std::numeric_limits<float>::epsilon())
		{
			// Ray is parallel to slab
			if (origin[i] < aabbMin[i] || origin[i] > aabbMax[i]) return false;
		}
		else
		{
			float ood = 1.0f / direction[i];
			float t1 = (aabbMin[i] - origin[i]) * ood;
			float t2 = (aabbMax[i] - origin[i]) * ood;

			if (t1 > t2) // T1 intersects with near plane, t2 with far
			{
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			// Compute intersection of slab intersection intervals
			if (t1 > tMin) tMin = t1;
			if (t2 > tMax) tMax = t2;

			if (tMin > tMax) return false; // No collision
		}
	}

	// Ray intersects all 3 slabs
	q = origin + direction * tMin;
	return true;
}