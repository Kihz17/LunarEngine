#include "WanderCondition.h"
#include "GLCommon.h"
#include "Components.h"

#include <iostream>

WanderCondition::WanderCondition(WanderBehaviour* behaviour, float useTime, float waitTime)
	: behaviour(behaviour),
	useTime(useTime),
	waitTime(waitTime),
	startTime(0.0f),
	endTime(0.0f)
{

}

WanderCondition::~WanderCondition()
{

}

bool WanderCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	float currentTime = (float) glfwGetTime();
	if (currentTime - endTime >= waitTime) return true; // We have waited long enough, use it again

	return false;
}

bool WanderCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	float currentTime = (float) glfwGetTime();
	if (currentTime - startTime >= useTime) return false; // Stop using if we have been idle for our wait time

	return true;
}

void WanderCondition::OnStart()
{
	startTime = (float) glfwGetTime();
}

void WanderCondition::OnStop()
{
	endTime = (float)glfwGetTime();
}

void WanderCondition::Update(float deltaTime)
{

}