#include "FleeCondition.h"
#include "Utils.h"
#include "Components.h"

#include <iostream>

FleeCondition::FleeCondition(FleeBehaviour* behaviour)
	: behaviour(behaviour)
{

}

FleeCondition::~FleeCondition()
{

}

bool FleeCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = FindTarget(entities);
	if (foundTarget) behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Only start if we found a target
}

bool FleeCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* target = behaviour->GetTarget();
	if (!behaviour->GetTarget()) // We have no target anymore, look for a new one
	{
		target = FindTarget(entities);
	}
	else // We have a target! Verify that it is still looking away from us
	{
		PositionComponent* posComp = target->GetComponent<PositionComponent>();
		RotationComponent* rotComp = target->GetComponent<RotationComponent>();
		glm::vec3 playerDir = glm::rotate(rotComp->value, -Utils::FrontVec());
		glm::vec3 seekDir = glm::normalize(posComp->value - behaviour->GetRigidBody()->GetPosition());
		if (glm::dot(playerDir, glm::normalize(behaviour->GetRigidBody()->GetPosition())) <= 0.0f) // Not looking away, search for a new target
		{
			target = FindTarget(entities);
		}
	}

	return target; // Keep running if we still have a target
}

void FleeCondition::OnStart()
{

}

void FleeCondition::OnStop()
{

}

void FleeCondition::Update(float deltaTime)
{

}

Entity* FleeCondition::FindTarget(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	std::unordered_map<unsigned int, Entity*>::const_iterator it;
	for (it = entities.begin(); it != entities.end(); it++)
	{
		Entity* entity = it->second;
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag("player")) continue; // Verify that the entity is a player

		RotationComponent* rotComp = entity->GetComponent<RotationComponent>();
		if (!rotComp) continue; // We didn't have a rotation component for some reason????

		PositionComponent* posComp = entity->GetComponent<PositionComponent>();
		if (!posComp) continue; // We didn't have a position component for some reason????

		// Check if player is looking away
		glm::vec3 playerDir = glm::rotate(rotComp->value, -Utils::FrontVec());
		glm::vec3 seekDir = glm::normalize(posComp->value - behaviour->GetRigidBody()->GetPosition());
		float dot = glm::dot(playerDir, seekDir);
		if (dot >= 0.0f) continue; // The player is not looking away from us

		glm::vec3 difference = behaviour->GetRigidBody()->GetPosition() - posComp->value;
		float distance = glm::length(difference);
		if (closestTarget == -1.0f || distance < closestTarget) // We found a new closer target!
		{
			foundTarget = entity;
			closestTarget = distance;
		}
	}

	return foundTarget;
}