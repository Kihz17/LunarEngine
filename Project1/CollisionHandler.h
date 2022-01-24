#pragma once

#include <Shapes.h>

#include <vector>

class RigidBody;

struct CollidingBodies
{
	CollidingBodies(RigidBody* bodyA, RigidBody* bodyB)
		: bodyA(bodyA),
		bodyB(bodyB) {}

	RigidBody* bodyA;
	RigidBody* bodyB;
};

class CollisionHandler
{
public:
	CollisionHandler() = default;
	~CollisionHandler() = default;

	bool CollideSphereSphere(RigidBody* bodyA, Physics::SphereShape* sphereA, RigidBody* bodyB, Physics::SphereShape* sphereB, float deltaTime);
	bool CollideSpherePlane(RigidBody* bodyA, Physics::SphereShape* sphere, RigidBody* bodyB, Physics::PlaneShape* plane, float deltaTime);
	
	void Collide(float deltaTime, std::vector<RigidBody*>& rigidBodies, std::vector<CollidingBodies>& collidingBodies);
};