#pragma once

#include "Component.h"

#include <glm/gtx/quaternion.hpp>
#include <vector>

enum EasingType
{
	EaseIn,
	EaseOut,
	EaseInOut,
	None
};

#define DIRECT_ROTATION 0
#define LERP_ROTATION 1
#define SLERP_ROTATION 2

struct KeyFramePositionComponent : Component
{
	KeyFramePositionComponent() = default;
	KeyFramePositionComponent(float time, const glm::vec3& position, EasingType easingType = EasingType::None)
		: time(time),
		position(position),
		easingType(easingType)
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::vec3 position;
	EasingType easingType;
};

struct KeyFrameScaleComponent : Component
{
	KeyFrameScaleComponent() = default;
	KeyFrameScaleComponent(float time, const glm::vec3& scale, EasingType easingType = EasingType::None)
		: time(time),
		scale(scale),
		easingType(easingType)
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::vec3 scale;
	EasingType easingType;
};

struct KeyFrameRotationComponent : Component
{
	KeyFrameRotationComponent() = default;
	KeyFrameRotationComponent(float time, const glm::quat& rotation, int interpolationType = 0, EasingType easingType = EasingType::None)
		: time(time),
		rotation(rotation),
		interpolationType(interpolationType),
		easingType(easingType)
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::quat rotation;
	EasingType easingType;
	int interpolationType;
};

struct AnimationComponent : Component
{
	AnimationComponent() = default;

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Animation"))
		{
			ImGui::DragFloat("Speed", &speed, 0.01);

			ImGui::NewLine();
			ImGui::Checkbox("Playing", &playing);

			ImGui::TreePop();
		}
	}

	bool playing;
	float duration;
	float currentTime;
	float speed;
	bool repeat;
	std::vector<KeyFramePositionComponent> keyFramePositions;
	std::vector<KeyFrameScaleComponent> keyFrameScales;
	std::vector<KeyFrameRotationComponent> keyFrameRotations;
};