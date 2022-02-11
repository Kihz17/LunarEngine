#include "AABB.h"

#include "GLCommon.h"

AABB::AABB(const glm::vec3& min, const glm::vec3 max) 
	: center(max + min * 0.5f),
	size(glm::vec3(max.x - center.x, max.y - center.y, max.z - center.z))
{

}

AABB::AABB(const glm::vec3& center, float sizeX, float sizeY, float sizeZ)
	: center(center),
	size(glm::vec3(sizeX, sizeY, sizeZ))
{

}

AABB::~AABB()
{

}

bool AABB::IsOnFrustum(const Frustum& frustum, const glm::mat4& transform) const
{
	const glm::vec3 transformedCenter = transform * glm::vec4(center, 1.0f);

	// Scaled orientation
	const glm::vec3 xPlane = glm::vec3(transform[0]) * size;
	const glm::vec3 yPlane = glm::vec3(transform[1]) * size;
	const glm::vec3 zPlane = glm::vec3(transform[2]) * size;

	const float newSizeX = std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), xPlane)) +
		std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), yPlane)) +
		std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), zPlane));

	const float newSizeY = std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), xPlane)) +
		std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), yPlane)) +
		std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), zPlane));

	const float newSizeZ = std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), xPlane)) +
		std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), yPlane)) +
		std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), zPlane));

	const AABB transformedBoundingBox(transformedCenter, newSizeX, newSizeY, newSizeZ);

	return transformedBoundingBox.IsOnOrForwardPlan(frustum.left) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.right) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.top) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.bottom) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.near) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.far);
}

bool AABB::IsOnOrForwardPlan(const Plan& plan) const // Taken from https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
{
	// Compute projection interval radius of b onto L(t) = b.x * p.n
	const float c = size.x * std::abs(plan.normal.x) + size.y * std::abs(plan.normal.y) + size.z * std::abs(plan.normal.z);
	return -c <= plan.GetSignedDistanceToPlan(center);
}

glm::vec3 AABB::GetMin() const
{
	return center - size;
}

glm::vec3 AABB::GetMax() const
{
	return center + size;
}