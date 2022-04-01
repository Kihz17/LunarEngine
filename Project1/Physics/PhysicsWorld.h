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

class PhysicsWorld : public Physics::IPhysicsWorld
{
public:
	virtual ~PhysicsWorld();

	virtual void SetGravity(const glm::vec3& gravity) override;
	virtual void RegisterCollisionListener(Physics::ICollisionListener* listener) override;

	virtual void AddBody(Physics::ICollisionBody* body) override;
	virtual void RemoveBody(Physics::ICollisionBody* body) override;

	virtual void Update(float deltaTime) override;

	btDiscreteDynamicsWorld* GetBulletWorld() { return world; }

private:
	friend class PhysicsFactory;

	PhysicsWorld();

	btCollisionConfiguration* configuration;
	btDispatcher* dispatcher;
	btBroadphaseInterface* broadfaceInterface;
	btConstraintSolver* constraintSolver;
	btSoftBodySolver* softBodySolver;

	btDiscreteDynamicsWorld* world;

	std::vector<Physics::ICollisionBody*> bodies;

	PhysicsWorld(const PhysicsWorld& other) {}
	PhysicsWorld& operator=(const PhysicsWorld& other) { return *this; }
};