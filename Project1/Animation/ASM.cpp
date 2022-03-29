#include "ASM.h"
#include <GLFW/glfw3.h>

ASM::ASM(SkeletalAnimationComponent* comp)
	: animComp(comp),
	currentState(nullptr)
{

}

bool ASM::SetState(const AnimationState& state, float speed)
{
	float currentTime = glfwGetTime();
	if (currentState && !currentState->cancellable && currentTime - beginTime < currentState->duration) return false; // Can't change state, we're locked in to this animation

	beginTime = currentTime;
	currentState = &state;
	animComp->speed = speed;
	animComp->SetAnimation(state.anim);
	return true;
}

void ASM::Update(float deltaTime)
{

}

bool ASM::IsAnimationLocked() const
{
	return currentState && !currentState->cancellable && glfwGetTime() - beginTime < currentState->duration;
}

bool ASM::CanMove() const
{
	return !currentState || currentState->canMove || !currentState->cancellable && glfwGetTime() - beginTime >= currentState->duration;
}