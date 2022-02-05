#include "IdleCondition.h"
#include "GLCommon.h"
#include "Components.h"

#include <iostream>

IdleCondition::IdleCondition(IdleBehaviour* behaviour, float useTime, float waitTime)
	: behaviour(behaviour),
	useTime(useTime),
	waitTime(waitTime),
	startTime(0.0f),
	endTime(0.0f)
{

}

IdleCondition::~IdleCondition()
{

}

bool IdleCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	float currentTime = (float)glfwGetTime();
	if (currentTime - endTime >= waitTime) return true; // We have waited long enough, use it again
}

bool IdleCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	float currentTime = (float)glfwGetTime();
	if (currentTime - startTime >= useTime) return false; // Stop using if we have been idle for our wait time

	return true;
}

void IdleCondition::OnStart()
{
	startTime = (float)glfwGetTime();
}

void IdleCondition::OnStop()
{
	endTime = (float)glfwGetTime();
}

void IdleCondition::Update(float deltaTime)
{

}