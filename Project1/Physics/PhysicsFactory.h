#pragma once

#include <IPhysicsFactory.h>

class Entity;
class PhysicsFactory : public Physics::IPhysicsFactory<Entity>
{
public:
	PhysicsFactory() = default;
	virtual ~PhysicsFactory() = default;

	virtual Physics::IPhysicsWorld<Entity>* CreateWorld() override;
	virtual Physics::IRigidBody* CreateRigidBody(const Physics::RigidBodyInfo& info, Physics::IShape* shape) override;
};