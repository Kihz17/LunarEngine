#pragma once

#include <ICollisionListener.h>

class Entity;
class CollisionListener : public Physics::ICollisionListener<Entity>
{
public:
	CollisionListener() = default;
	virtual ~CollisionListener() = default;

	virtual void Collide(CollisionEvent& collisionEvent) override;
};