#pragma once

#include <ICollisionListener.h>

class CollisionListener : public Physics::ICollisionListener
{
public:
	CollisionListener() = default;
	virtual ~CollisionListener() = default;

	virtual void Collide(Physics::IRigidBody* bodyA, Physics::IRigidBody* bodyB) override;
};