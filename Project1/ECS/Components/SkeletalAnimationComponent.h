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
		lastAnim(nullptr),
		currentTime(0.0f),
		lastTime(0.0f),
		speed(1.0f),
		repeat(true),
		lerping(false),
		lerpSpeed(5.0f),
		allowRootMotion(false)
	{
		boneMatrices.reserve(Animation::MAX_BONES); // It's likely that the amount of bones is generally high, might as well avoid some re-allocations

		for (int i = 0; i < 100; i++) boneMatrices.push_back(glm::mat4(1.0f)); // Default to identity matrix
	}

	void SetAnimation(Animation* animation, bool lerp = true)
	{
		if (animation == anim) return;

		lastAnim = anim;
		anim = animation;

		if (lerp && lastAnim) // We want to interpolate between the 2 animations
		{
			lastTime = currentTime;
			lerping = true;
			lerpTime = 0.0f;
		}

		currentTime = 0.0f;
	}

	float speed;
	bool repeat;
	float lerpSpeed;
	bool allowRootMotion;

private:
	friend class SkeletalAnimationLayer;
	friend class GameEngine;

	Animation* anim;
	Animation* lastAnim;
	float currentTime;
	float lastTime;
	std::vector<glm::mat4> boneMatrices;

	float lerpTime;
	bool lerping;
};