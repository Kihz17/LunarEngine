#include "EvadeCondition.h"
#include "GLCommon.h"
#include "Components.h"

#include <iostream>

EvadeCondition::EvadeCondition(EvadeBehaviour* behaviour)
	: behaviour(behaviour)
{

}

EvadeCondition::~EvadeCondition()
{

}

bool EvadeCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* target = TryFindBullet(entities);
	if (target) behaviour->SetTarget(target);
	return target;
}

bool EvadeCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* target = behaviour->GetTarget();
	if (!target)  // We have no target anymore, look for a new one
	{
		target = TryFindBullet(entities);
	}
	else // We still have a target, check if it is still moving towards us
	{
		RigidBodyComponent* rigidComp = target->GetComponent<RigidBodyComponent>();
		glm::vec3 bulletVelocity = rigidComp->ptr->GetLinearVelocity();
		glm::vec3 bulletToEntity = behaviour->GetRigidBody()->GetPosition() - rigidComp->ptr->GetPosition();
		float dot = glm::dot(bulletVelocity, bulletToEntity);
		if (dot <= 0.0f) // Bullet isn't even travelling in our general direction
		{
			target = TryFindBullet(entities); // Try to find new target
		}
	}

	behaviour->SetTarget(target);
	return target;
}

void EvadeCondition::OnStart()
{

}

void EvadeCondition::OnStop()
{

}

void EvadeCondition::Update(float deltaTime)
{

}

Entity* EvadeCondition::TryFindBullet(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	std::unordered_map<unsigned int, Entity*>::const_iterator it;
	for (it = entities.begin(); it != entities.end(); it++)
	{
		Entity* entity = it->second;
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag("bullet")) continue; // Verify that the entity is a bullet

		RigidBodyComponent* rigidComp = entity->GetComponent<RigidBodyComponent>();
		if (!rigidComp) continue; // We didn't have a rigid body component for some reason????

		glm::vec3 bulletVelocity = rigidComp->ptr->GetLinearVelocity();
		glm::vec3 bulletToEntity = behaviour->GetRigidBody()->GetPosition() - rigidComp->ptr->GetPosition();
		float dot = glm::dot(bulletVelocity, bulletToEntity);
		//std::cout << "Dot: " << dot << "\n";
		if (dot <= 0.0f) continue; // Bullet isn't even travelling in our general direction

		float distance = glm::length(bulletToEntity);
		if (closestTarget == -1.0f || distance < closestTarget) // We found a new closer target!
		{
			foundTarget = entity;
			closestTarget = distance;
		}
	}

	return nullptr;
}