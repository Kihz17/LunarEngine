#pragma once

#include "Component.h"
#include "Animation.h"

#include <glm/glm.hpp>

#include <vector>

class SkeletalAnimationComponent : public Component
{
public:
	SkeletalAnimationComponent()
		: anim(nullptr),
		currentTime(0.0f),
		speed(1.0f),
		repeat(true)
	{
		boneMatrices.reserve(Animation::MAX_BONES); // It's likely that the amount of bones is generally high, might as well avoid some re-allocations

		for (int i = 0; i < 100; i++) boneMatrices.push_back(glm::mat4(1.0f)); // Default to identity matrix
	}

	virtual void ImGuiUpdate() override { }

	void SetAnimation(Animation* animation)
	{
		anim = animation;
		currentTime = 0.0f;
	}

	float speed;
	bool repeat;

private:
	friend class SkeletalAnimationLayer;
	friend class GameEngine;

	Animation* anim;
	float currentTime;
	std::vector<glm::mat4> boneMatrices;
};