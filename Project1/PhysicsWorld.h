#pragma once

#include "CollisionHandler.h"
#include "RigidBody.h"

#include <IPhysicsWorld.h>
#include <ICollisionListener.h>

#include <vector>

class PhysicsWorld : public Physics::IPhysicsWorld
{
public:
	PhysicsWorld();
	virtual ~PhysicsWorld();

	virtual void SetGravity(const glm::vec3& gravity) override { this->gravity = gravity; }
	virtual void RegisterCollisionListener(Physics::ICollisionListener* listener) override { collisionListener = listener; }
	virtual void AddRigidBody(Physics::IRigidBody* body) override;
	virtual void RemoveRigidBody(Physics::IRigidBody* body) override;
	virtual void Update(float deltaTime) override;

private:
	glm::vec3 gravity;
	std::vector<RigidBody*> rigidBodies;
	CollisionHandler* collisionHandler;
	Physics::ICollisionListener* collisionListener;

	PhysicsWorld(const PhysicsWorld& other) {}
	PhysicsWorld& operator=(const PhysicsWorld& other) { return *this; }
};