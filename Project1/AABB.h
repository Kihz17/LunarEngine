#pragma once

#include "IBoundingVolume.h"

#include <glm/glm.hpp>

class AABB : public IBoundingVolume
{
public:
	AABB() = default;
	AABB(const glm::vec3& min, const glm::vec3 max);
	AABB(const glm::vec3& center, float sizeX, float sizeY, float sizeZ);
	virtual ~AABB();

	virtual bool IsOnFrustum(const Frustum& frustum, const glm::mat4& transform) const override;
	virtual bool IsOnOrForwardPlan(const Plan& plan) const override;

	glm::vec3 GetMin() const;
	glm::vec3 GetMax() const;

	glm::vec3 center;
	glm::vec3 size;
};