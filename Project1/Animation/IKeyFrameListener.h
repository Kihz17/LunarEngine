#pragma once 

#include "Components.h"
#include <glm/glm.hpp>

enum class KeyFrameType
{
	Position,
	Scale,
	Rotation
};

struct KeyFrame
{
	float time;
	KeyFrameType type;
	EasingType easingType;
};

class IKeyFrameListener
{
public:
	virtual ~IKeyFrameListener() = default;

	virtual void OnKeyFrame(const KeyFrame& frame) = 0;
};