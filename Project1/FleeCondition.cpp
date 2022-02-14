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

bool FleeCondition::CanUse(const std::vector<Entity*>& entities)
{
	Entity* foundTarget = FindTarget(entities);
	if (foundTarget) behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Only start if we found a target
}

bool FleeCondition::CanContinueToUse(const std::vector<Entity*>& entities)
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
		glm::vec3 playerDir = glm::rotate(rotComp->value, Utils::FrontVec());
		glm::vec3 seekDir = glm::normalize(posComp->value - behaviour->GetRigidBody()->GetPosition());
		if (glm::dot(playerDir, glm::normalize(behaviour->GetRigidBody()->GetPosition())) <= 0.0f) // Not looking away, search for a new target
		{
			target = FindTarget(entities);
		}
	}

	behaviour->SetTarget(target);
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

Entity* FleeCondition::FindTarget(const std::vector<Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	for (Entity* entity : entities)
	{
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag("player")) continue; // Verify that the entity is a player

		RigidBodyComponent* rigidComp = entity->GetComponent<RigidBodyComponent>();

		// Check if player is looking away
		glm::vec3 playerDir = glm::rotate(rigidComp->ptr->GetOrientation(), Utils::FrontVec());
		glm::vec3 seekDir = glm::normalize(rigidComp->ptr->GetPosition() - behaviour->GetRigidBody()->GetPosition());
		float dot = glm::dot(playerDir, seekDir);
		if (dot >= 0.0f) continue; // The player is not looking away from us

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