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
	EntityManager& entityManager;
};