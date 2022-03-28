#pragma once

#include "ApplicationLayer.h"
#include "EntityManager.h"
#include "Camera.h"
#include "RigidBody.h"

#include "PhysicsWorld.h"

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class PlayerController : public ApplicationLayer
{
public:
	PlayerController(Camera& camera, EntityManager& entityManager, PhysicsWorld* physicsWorld);

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	EntityManager& entityManager;
	PhysicsWorld* physicsWorld;
	Camera& camera;

	Entity* playerEntity;
	btKinematicCharacterController* btController;
	btPairCachingGhostObject* ghostObj;
};