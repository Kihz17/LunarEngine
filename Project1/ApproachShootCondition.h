#pragma once

#include "ISteeringCondition.h"
#include "SeekBehaviour.h"
#include "EntityManager.h"
#include "Mesh.h"

#include <IPhysicsFactory.h>

class ApproachShootCondition : public ISteeringCondition
{
public:
	ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval, EntityManager& entityManager,
		Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, Mesh* bulletMesh);
	virtual ~ApproachShootCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	Entity* FindTarget(const std::vector<Entity*>& entities);
	Entity* SpawnBullet(const glm::vec3& position, const glm::vec3& direction);

	SeekBehaviour* behaviour;
	float lastShootTime;
	float shootInterval;

	EntityManager& entityManager;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;
	Mesh* bulletMesh;
};