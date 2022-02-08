#pragma once

#include "ApplicationLayer.h"
#include "Camera.h"
#include "Renderer.h"
#include "EntityManager.h"
#include "Mesh.h"
#include "Key.h"

#include <IPhysicsFactory.h>
#include <glm/glm.hpp>

class PlayerController : public ApplicationLayer
{
public:
	PlayerController(Camera& camera, Entity* entity, const WindowSpecs& windowSpecs, EntityManager& entityManager, 
		Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, Mesh* bulletMesh);
	virtual ~PlayerController();

	virtual void OnUpdate(float deltaTime) override;

private:
	Entity* SpawnBullet(const glm::vec3& position, const glm::vec3& direction);

	Camera& camera;
	Entity* entity;
	const WindowSpecs& windowSpecs;
	glm::vec2 lastCursorPos;
	float moveSpeed;

	EntityManager& entityManager;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;
	Mesh* bulletMesh;

	Key* wKey;
	Key* aKey;
	Key* sKey;
	Key* dKey;
	Key* escKey;
	Key* lmbKey;
};