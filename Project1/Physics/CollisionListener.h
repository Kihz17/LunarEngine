#pragma once

#include "EntityManager.h"
#include <ICollisionListener.h>

class Entity;
class CollisionListener : public Physics::ICollisionListener<Entity>
{
public:
	CollisionListener(EntityManager& entityManager);
	virtual ~CollisionListener() = default;

	virtual void Collide(CollisionEvent& collisionEvent) override;

private:
	void ResetGame(Physics::IPhysicsWorld<Entity>* physicsWorld);
	void RemoveEntity(Physics::IRigidBody* body, Entity* entity, Physics::IPhysicsWorld<Entity>* physicsWorld);

	EntityManager& entityManager;
};