#pragma once

#include "ApplicationLayer.h"
#include "EntityManager.h"
#include "Mesh.h"

#include <IPhysicsFactory.h>
#include <glm/glm.hpp>

class AILayer;
class EnemySpawner : public ApplicationLayer
{
public:
	EnemySpawner(float spawnInterval, float spawnRadius, EntityManager& entityManager, 
		Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, Mesh* enemyMesh, AILayer* aiLayer);
	virtual ~EnemySpawner();

	virtual void OnUpdate(float deltaTime) override;

private:
	void SpawnTypeA();
	void SpawnTypeB();
	void SpawnTypeC();
	void SpawnTypeD();
	Entity* SpawnPhysicsSphere(const glm::vec3& color);

	float spawnInterval;
	float spawnRadius;
	float lastSpawnTime;
	EntityManager& entityManager;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;
	Mesh* enemyMesh;
	AILayer* aiLayer;
};