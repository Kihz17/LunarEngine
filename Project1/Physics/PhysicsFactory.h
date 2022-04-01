#pragma once

#include "Mesh.h"
#include "BulletUtils.h"

#include <IPhysicsFactory.h>

#include <Bullet/btBulletDynamicsCommon.h>

#include <glm/glm.hpp>

#include <unordered_map>

// TODO: Track bullet collision shapes to not duplicate them when making new rigidbodies

class PhysicsFactory : public Physics::IPhysicsFactory
{
public:
	PhysicsFactory();
	virtual ~PhysicsFactory();

	virtual Physics::IPhysicsWorld* CreateWorld() override;

	virtual Physics::IRigidBody* CreateRigidBody(const Physics::RigidBodyInfo& info, Physics::IShape* shape) override;

	static Physics::MeshShape* GetMeshShape(IMesh* mesh);

private:
	static std::unordered_map<IMesh*, Physics::MeshShape*> meshCoilliders;
};