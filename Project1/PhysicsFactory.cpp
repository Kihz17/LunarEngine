#include "PhysicsFactory.h"

#include "RigidBody.h"
#include "PhysicsWorld.h"

Physics::IPhysicsWorld* PhysicsFactory::CreateWorld()
{
	return new PhysicsWorld();
}

Physics::IRigidBody* PhysicsFactory::CreateRigidBody(const Physics::RigidBodyInfo& info, Physics::IShape* shape)
{
	return new RigidBody(info, shape);
}