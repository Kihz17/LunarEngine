#include "FlockingCondition.h"

#include <iostream>

FlockingCondition::FlockingCondition(FlockingBehaviour* behaviour)
	: behaviour(behaviour),
	canUse(false)
{

}

FlockingCondition::~FlockingCondition()
{

}

bool FlockingCondition::CanUse(const std::vector<Entity*>& entities)
{
	return canUse;
}

bool FlockingCondition::CanContinueToUse(const std::vector<Entity*>& entities)
{
	return canUse;
}

void FlockingCondition::OnStart()
{

}

void FlockingCondition::OnStop()
{

}

void FlockingCondition::Update(float deltaTime)
{

}