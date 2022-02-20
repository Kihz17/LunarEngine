#include "FormationCondition.h"

#include <iostream>

FormationCondition::FormationCondition(FormationBehaviour* behaviour)
	: behaviour(behaviour)
{

}

FormationCondition::~FormationCondition()
{

}

bool FormationCondition::CanUse(const std::vector<Entity*>& entities)
{
	return true;
}

bool FormationCondition::CanContinueToUse(const std::vector<Entity*>& entities)
{
	return true;
}

void FormationCondition::OnStart()
{

}

void FormationCondition::OnStop()
{

}

void FormationCondition::Update(float deltaTime)
{

}