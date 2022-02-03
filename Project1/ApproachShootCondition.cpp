#include "ApproachShootCondition.h"
#include "GLCommon.h"
#include "Components.h"

#include <iostream>

ApproachShootCondition::ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval)
	: behaviour(behaviour),
	shootInterval(shootInterval),
	lastShootTime(-1.0f)
{

}

ApproachShootCondition::~ApproachShootCondition()
{

}

bool ApproachShootCondition::CanUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	
	Entity* foundTarget = FindTarget(entities);
	if(foundTarget) behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Only start if we found a target
}

bool ApproachShootCondition::CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities)
{
	if (behaviour->GetTarget()) return true; // We still have a target, keep running

	// Try to find a target
	Entity* foundTarget = FindTarget(entities);
	if (foundTarget) behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Keep running if we found a new target
}

void ApproachShootCondition::OnStart()
{

}

void ApproachShootCondition::OnStop()
{

}

void ApproachShootCondition::Update(float deltaTime)
{
	if (!behaviour->IsWithinRadius()) return; // Not within radius, don't even try to shoot

	float currentTime = (float) glfwGetTime();
	if (lastShootTime == -1.0f || (currentTime - lastShootTime) >= shootInterval) // We can shoot!
	{
		// TODO: Shoot the thing
		lastShootTime = currentTime;
		std::cout << "Shoot!\n";
	}
}

Entity* ApproachShootCondition::FindTarget(const std::unordered_map<unsigned int, Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	std::unordered_map<unsigned int, Entity*>::const_iterator it;
	for (it = entities.begin(); it != entities.end(); it++)
	{
		Entity* entity = it->second;
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag("player")) continue; // Verify that the entity is a player

		PositionComponent* posComp = entity->GetComponent<PositionComponent>();
		if (!posComp) continue; // We didn't have a position component for some reason????

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