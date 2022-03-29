#pragma once

#include "Animation.h"
#include "SkeletalAnimationComponent.h"

struct AnimationState
{
	Animation* anim = nullptr;
	float duration = 0.0f;
	bool cancellable = true;
	bool canMove = true;
};

class ASM
{
public:
	ASM(SkeletalAnimationComponent* comp);

	bool SetState(const AnimationState& state, float speed);
	void Update(float deltaTime);
	bool IsAnimationLocked() const;
	bool CanMove() const;

	SkeletalAnimationComponent* animComp;

private:
	float beginTime;
	const AnimationState* currentState;
};