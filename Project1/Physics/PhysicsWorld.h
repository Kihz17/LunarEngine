#pragma once

#include "CollisionHandler.h"
#include "RigidBody.h"

#include <IPhysicsWorld.h>
#include <ICollisionListener.h>

#include <vector>
#include <unordered_map>

template <class T> class ICollisionListener;
class Entity;
class PhysicsWorld : public Physics::IPhysicsWorld<Entity>
{
public:
	PhysicsWorld();
	virtual ~PhysicsWorld();

	virtual void SetGravity(const glm::vec3& gravity) override { this->gravity = gravity; }
	virtual void RegisterCollisionListener(Physics::ICollisionListener<Entity>* listener) override { collisionListeners.push_back(listener); }
	virtual void AddRigidBody(Physics::IRigidBody* body, Entity* entity) override;
	virtual void RemoveRigidBody(Physics::IRigidBody* body) override;
	virtual Entity* GetRigidBodyOwner(Physics::IRigidBody* body) override;
	virtual void Update(float deltaTime) override;

private:
	glm::vec3 gravity;
	std::unordered_map<RigidBody*, Entity*> rigidBodiesToOwner;
	std::vector<RigidBody*> rigidBodies;
	std::vector<RigidBody*> rigidBodiesToRemove;
	CollisionHandler* collisionHandler;
	std::vector<Physics::ICollisionListener<Entity>*> collisionListeners;

	PhysicsWorld(const PhysicsWorld& other) {}
	PhysicsWorld& operator=(const PhysicsWorld& other) { return *this; }
};