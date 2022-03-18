#pragma once

#include <ICollisionListener.h>
#include <Shapes.h>

#include <vector>

class RigidBody;

class PhysicsWorld;
struct CollidingBodies
{
	CollidingBodies(RigidBody* bodyA, RigidBody* bodyB)
		: bodyA(bodyA),
		bodyB(bodyB) {}

	RigidBody* bodyA;
	RigidBody* bodyB;
};

class Entity;
class CollisionHandler
{
public:
	CollisionHandler() = default;
	~CollisionHandler() = default;

	bool CollideSphereSphere(RigidBody* bodyA, Physics::SphereShape* sphereA, RigidBody* bodyB, Physics::SphereShape* sphereB, float deltaTime, 
		std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld);
	bool CollideSpherePlane(RigidBody* bodyA, Physics::SphereShape* sphere, RigidBody* bodyB, Physics::PlaneShape* plane, float deltaTime, 
		std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld);
	bool CollideSphereAABB(RigidBody* bodyA, Physics::SphereShape* sphere, RigidBody* bodyB, Physics::AABBShape* aabb, float deltaTime,
		std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld);
	bool CollidePlaneAABB(RigidBody* bodyA, Physics::PlaneShape* plane, RigidBody* bodyB, Physics::AABBShape* aabb, float deltaTime,
		std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld);
	
	void Collide(float deltaTime, std::vector<RigidBody*>& rigidBodies, std::vector<CollidingBodies>& collidingBodies, std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld);
};