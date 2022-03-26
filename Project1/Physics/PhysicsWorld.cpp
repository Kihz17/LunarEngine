#include "PhysicsWorld.h"

#include <iostream>

PhysicsWorld::PhysicsWorld()
	: IPhysicsWorld(),
	configuration(new btDefaultCollisionConfiguration()),
	dispatcher(new btCollisionDispatcher(configuration)),
	broadfaceInterface(new btDbvtBroadphase()),
	constraintSolver(new btSequentialImpulseConstraintSolver()),
	softBodySolver(new btDefaultSoftBodySolver()),
	world(new btSoftRigidDynamicsWorld(dispatcher, broadfaceInterface, constraintSolver, configuration, softBodySolver))
{

}

PhysicsWorld::~PhysicsWorld()
{
	delete world;
}

void PhysicsWorld::SetGravity(const glm::vec3& gravity)
{
	world->setGravity(BulletUtils::GLMVec3ToBullet(gravity));
}

void PhysicsWorld::AddRigidBody(Physics::IRigidBody* body, Entity* owner)
{
	if (!body) return;

	RigidBody* rb = dynamic_cast<RigidBody*>(body);
	if (!rb) return;

	rigidBodiesToOwner.insert({ rb , owner });
	world->addRigidBody(rb->GetBulletBody());
}

void PhysicsWorld::RemoveRigidBody(Physics::IRigidBody* body)
{
	RigidBody* rigidBody = dynamic_cast<RigidBody*>(body);
	rigidBodiesToOwner.erase(rigidBody);
	world->removeRigidBody(rigidBody->GetBulletBody());
}

Entity* PhysicsWorld::GetRigidBodyOwner(Physics::IRigidBody* body)
{
	std::unordered_map<RigidBody*, Entity*>::iterator it = rigidBodiesToOwner.find(dynamic_cast<RigidBody*>(body));
	if (it == rigidBodiesToOwner.end()) return nullptr;
	return it->second;
}

void PhysicsWorld::Update(float deltaTime)
{
	world->stepSimulation(deltaTime);
}