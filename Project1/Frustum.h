#pragma once

#include "Camera.h"

#include <glm/glm.hpp>

// Represents a plant in a frustum. I frustum has 6 plans: near, far, right, left, top, and bottom
// This will help identify if objects are within a frustum. Will mostly be used for frustum culling
struct Plan
{
	Plan() = default;
	Plan(const glm::vec3& p1, const glm::vec3& normal)
		: normal(glm::normalize(normal)),
		distance(glm::dot(normal, p1))
	{}

	float GetSignedDistanceToPlan(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}

	glm::vec3 normal;
	float distance; // distance from the origin to the nearest point in the plan
};

// Represents a frustum
struct Frustum
{
	Plan top;
	Plan bottom;

	Plan right;
	Plan left;

	Plan far;
	Plan near;
};

namespace FrustumUtils
{
	Frustum CreateFrustumFromCamera(const Camera& camera, const float aspect, const float far, const float near);
}