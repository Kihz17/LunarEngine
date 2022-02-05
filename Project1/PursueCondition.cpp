#include "PursueCondition.h"
#include "Utils.h"
#include "Components.h"

#include <iostream>

PursueCondition::PursueCondition(PursueBehaviour* behaviour)
	: behaviour(behaviour)
{

}

PursueCondition::~PursueCondition()
{

}

bool PursueCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = FindTarget(entities);
	behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Only start if we found a target
}

bool PursueCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* target = behaviour->GetTarget();
	if (!target) // We have no target anymore, look for a new one
	{
		target = FindTarget(entities);
	}
	else // We have a target! Verify that it is still looking away from us
	{
		PositionComponent* posComp = target->GetComponent<PositionComponent>();
		RotationComponent* rotComp = target->GetComponent<RotationComponent>();
		glm::vec3 playerDir = glm::rotate(rotComp->value, -Utils::FrontVec());
		glm::vec3 seekDir = glm::normalize(posComp->value - behaviour->GetRigidBody()->GetPosition());
		float dot = glm::dot(playerDir, seekDir);
		if (glm::dot(playerDir, seekDir) <= 0.0f) // Not looking away, search for a new target
		{
			target = FindTarget(entities);
		}
	}

	behaviour->SetTarget(target);
	return target; // Keep running if we still have a target
}

void PursueCondition::OnStart()
{

}

void PursueCondition::OnStop()
{

}

void PursueCondition::Update(float deltaTime)
{

}

Entity* PursueCondition::FindTarget(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	std::unordered_map<unsigned int, Entity*>::const_iterator it;
	for (it = entities.begin(); it != entities.end(); it++)
	{
		Entity* entity = it->second;
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag("player")) continue; // Verify that the entity is a player

		RigidBodyComponent* rigidComp = entity->GetComponent<RigidBodyComponent>();
		if (!rigidComp) continue; // We didn't have a rigid component for some reason????

		glm::vec3 difference = behaviour->GetRigidBody()->GetPosition() - rigidComp->ptr->GetPosition();
		float distance = glm::length(difference);
		if (closestTarget == -1.0f || distance < closestTarget) // We found a new closer target!
		{
			foundTarget = entity;
			closestTarget = distance;
		}
	}

	return foundTarget;
}