#pragma once

#include "Camera.h"

#include <glm/glm.hpp>

// Represents a plant in a frustum. I frustum has 6 plans: near, far, right, left, top, and bottom
// This will help identify if objects are within a frustum. Will mostly be used for frustum culling
struct Plane
{
	Plane() = default;
	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{}

	float GetSignedDistanceToPlan(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}

	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	float distance = 0.0f; // distance from the origin to the nearest point in the plan
};

// Represents a frustum
struct Frustum
{
	Plane top;
	Plane bottom;

	Plane right;
	Plane left;

	Plane far;
	Plane near;
};

namespace FrustumUtils
{
	Frustum CreateFrustumFromCamera(const Camera& camera, const float aspect, const float far, const float near);
}