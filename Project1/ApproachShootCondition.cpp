#include "ApproachShootCondition.h"
#include "GLCommon.h"
#include "Components.h"
#include "UUID.h"

#include <Shapes.h>

#include <iostream>

ApproachShootCondition::ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval, EntityManager& entityManager,
	Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, Mesh* bulletMesh)
	: behaviour(behaviour),
	shootInterval(shootInterval),
	lastShootTime(-1.0f),
	entityManager(entityManager),
	physicsFactory(physicsFactory),
	physicsWorld(physicsWorld),
	bulletMesh(bulletMesh)
{

}

ApproachShootCondition::~ApproachShootCondition()
{

}

bool ApproachShootCondition::CanUse(const std::vector<Entity*>& entities)
{
	
	Entity* foundTarget = FindTarget(entities);
	if(foundTarget) behaviour->SetTarget(foundTarget); // We found a target, make sure to tell the behaviour who we are targeting
	return foundTarget; // Only start if we found a target
}

bool ApproachShootCondition::CanContinueToUse(const std::vector<Entity*>& entities)
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
		lastShootTime = currentTime;
		glm::vec3 spawnPos = behaviour->GetRigidBody()->GetPosition();
		spawnPos.y += 0.5f;
		glm::vec3 bulletDirection = glm::normalize(behaviour->GetTarget()->GetComponent<PositionComponent>()->value - spawnPos);
		spawnPos += glm::vec3(bulletDirection.x, 0.0f, bulletDirection.z) * 2.0f;
		SpawnBullet(spawnPos, bulletDirection);
	}
}

Entity* ApproachShootCondition::FindTarget(const std::vector<Entity*>& entities)
{
	Entity* foundTarget = nullptr;
	float closestTarget = -1.0f;
	for (Entity* entity : entities)
	{
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

Entity* ApproachShootCondition::SpawnBullet(const glm::vec3& position, const glm::vec3& direction)
{
	constexpr float bulletRadius = 0.2f;
	constexpr float bulletSpeed = 40.0f;
	Entity* physicsSphere = entityManager.CreateEntity(std::to_string(UUID()));
	physicsSphere->AddComponent<PositionComponent>();
	physicsSphere->AddComponent<ScaleComponent>(glm::vec3(bulletRadius, bulletRadius, bulletRadius));
	physicsSphere->AddComponent<RotationComponent>();
	TagComponent* tagComp = physicsSphere->AddComponent<TagComponent>();
	tagComp->AddTag("bullet");

	Physics::SphereShape* shape = new Physics::SphereShape(bulletRadius);

	// Rigid Body
	Physics::RigidBodyInfo rigidInfo;
	rigidInfo.linearDamping = 0.001f;
	rigidInfo.angularDamping = 0.001f;
	rigidInfo.isStatic = false;
	rigidInfo.mass = 0.01f;
	rigidInfo.position = position;
	rigidInfo.linearVelocity = direction * bulletSpeed;
	rigidInfo.restitution = 0.001f;
	Physics::IRigidBody* rigidBody = physicsFactory->CreateRigidBody(rigidInfo, shape);
	rigidBody->UseLocalGravity(true);
	rigidBody->SetGravityAcceleration(glm::vec3(0.0f, -2.5f, 0.0f));
	physicsSphere->AddComponent<RigidBodyComponent>(rigidBody);
	physicsWorld->AddRigidBody(physicsSphere->GetComponent<RigidBodyComponent>()->ptr, physicsSphere);

	// Render Info
	RenderComponent::RenderInfo sphereInfo;
	sphereInfo.mesh = bulletMesh;
	sphereInfo.isColorOverride = true;
	sphereInfo.colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);
	physicsSphere->AddComponent<RenderComponent>(sphereInfo);
	return physicsSphere;
}