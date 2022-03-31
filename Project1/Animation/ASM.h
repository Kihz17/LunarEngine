#pragma once

#include "Animation.h"
#include "SkeletalAnimationComponent.h"

struct AnimationState
{
	Animation* anim = nullptr;
	float duration = 0.0f;
	bool cancellable = true;
	float speed = 1.0f;
	int priority = 0;
};

class ASM
{
public:
	ASM(SkeletalAnimationComponent* comp);

	bool SetState(const AnimationState& state);

	bool CanPlayAnimation(const AnimationState& state) const;
	bool CanPlayPriority(int priority) const;

	float GetTimePlayed() const;

	Animation* GetAnimation();

	SkeletalAnimationComponent* animComp;

private:
	float beginTime;
	const AnimationState* currentState;
};