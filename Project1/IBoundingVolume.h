#pragma once

#include "Frustum.h"

class IBoundingVolume
{
public:
	virtual ~IBoundingVolume() = default;

	virtual bool IsOnFrustum(const Frustum& frustum, const glm::mat4& transform) const = 0;
	virtual bool IsOnOrForwardPlan(const Plan& plan) const = 0;

	bool IsOnFrustum(const Frustum& frustum) const
	{
		return IsOnOrForwardPlan(frustum.left) &&
			IsOnOrForwardPlan(frustum.right) &&
			IsOnOrForwardPlan(frustum.top) &&
			IsOnOrForwardPlan(frustum.bottom) &&
			IsOnOrForwardPlan(frustum.near) &&
			IsOnOrForwardPlan(frustum.far);
	}
};