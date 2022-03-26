#pragma once

#include "RigidBody.h"
#include "BulletUtils.h"

#include <IPhysicsWorld.h>
#include <ICollisionListener.h>

#include <Bullet/btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodySolvers.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>

#include <vector>
#include <unordered_map>

template <class T> class ICollisionListener;
class Entity;
class PhysicsWorld : public Physics::IPhysicsWorld<Entity>
{
public:
	PhysicsWorld();
	virtual ~PhysicsWorld();

	virtual void SetGravity(const glm::vec3& gravity) override;
	virtual void RegisterCollisionListener(Physics::ICollisionListener<Entity>* listener) override { collisionListeners.push_back(listener); }
	virtual void AddRigidBody(Physics::IRigidBody* body, Entity* entity) override;
	virtual void RemoveRigidBody(Physics::IRigidBody* body) override;
	virtual Entity* GetRigidBodyOwner(Physics::IRigidBody* body) override;
	virtual void Update(float deltaTime) override;

private:
	btCollisionConfiguration* configuration;
	btDispatcher* dispatcher;
	btBroadphaseInterface* broadfaceInterface;
	btConstraintSolver* constraintSolver;
	btSoftBodySolver* softBodySolver;

	btSoftRigidDynamicsWorld* world;

	std::unordered_map<RigidBody*, Entity*> rigidBodiesToOwner;
	std::vector<Physics::ICollisionListener<Entity>*> collisionListeners;

	PhysicsWorld(const PhysicsWorld& other) {}
	PhysicsWorld& operator=(const PhysicsWorld& other) { return *this; }
};