#include "ASM.h"
#include <GLFW/glfw3.h>

ASM::ASM(SkeletalAnimationComponent* comp)
	: animComp(comp),
	currentState(nullptr)
{

}

bool ASM::SetState(const AnimationState& state)
{
	if (!CanPlayAnimation(state)) return false; // Can't change state, we're locked in to this animation

	beginTime = glfwGetTime();
	currentState = &state;
	animComp->speed = state.speed;
	animComp->SetAnimation(state.anim);
	return true;
}

bool ASM::CanPlayAnimation(const AnimationState& state) const
{
	float currentTime = glfwGetTime();
	return !currentState || (state.priority > currentState->priority) 
		|| (state.priority == currentState->priority && currentState->cancellable)
		|| currentTime - beginTime >= currentState->duration;
}

bool ASM::CanPlayPriority(int priority) const
{
	return !currentState || (priority > currentState->priority) | (priority == currentState->priority && currentState->cancellable);
}

Animation* ASM::GetAnimation()
{
	if (!currentState) return nullptr;
	return currentState->anim;
}

float ASM::GetTimePlayed() const
{
	if (!currentState) return 0.0f;
	return currentState->duration == 0.0f ? 0.0f : glfwGetTime() - beginTime;
}