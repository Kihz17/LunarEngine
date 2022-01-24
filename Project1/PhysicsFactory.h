#pragma once

#include <IPhysicsFactory.h>

class PhysicsFactory : public Physics::IPhysicsFactory
{
public:
	PhysicsFactory() = default;
	virtual ~PhysicsFactory() = default;

	virtual Physics::IPhysicsWorld* CreateWorld() override;
	virtual Physics::IRigidBody* CreateRigidBody(const Physics::RigidBodyInfo& info, Physics::IShape* shape) override;
};