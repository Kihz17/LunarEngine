#include "PhysicsWorld.h"

#include <iostream>

PhysicsWorld::PhysicsWorld()
	: IPhysicsWorld(),
	configuration(new btDefaultCollisionConfiguration()),
	dispatcher(new btCollisionDispatcher(configuration)),
	broadfaceInterface(new btDbvtBroadphase()),
	constraintSolver(new btSequentialImpulseConstraintSolver()),
	softBodySolver(new btDefaultSoftBodySolver()),
	world(nullptr)
{
	world = new btDiscreteDynamicsWorld(dispatcher, broadfaceInterface, constraintSolver, configuration);
}

PhysicsWorld::~PhysicsWorld()
{
	delete world;
	delete configuration;
	delete dispatcher;
	delete broadfaceInterface;
	delete constraintSolver;
	delete softBodySolver;
}

void PhysicsWorld::SetGravity(const glm::vec3& gravity)
{
	world->setGravity(BulletUtils::GLMVec3ToBullet(gravity));
}

void PhysicsWorld::RegisterCollisionListener(Physics::ICollisionListener* listener)
{

}

void PhysicsWorld::AddBody(Physics::ICollisionBody* body)
{
	if (!body) return;

	if (body->GetBodyType() == Physics::CollisionBodyType::Soft)
	{

	}

	bodies.push_back(body);
	world->addRigidBody(dynamic_cast<RigidBody*>(body)->GetBulletBody());
}

void PhysicsWorld::RemoveBody(Physics::ICollisionBody* body)
{
	world->removeRigidBody(dynamic_cast<RigidBody*>(body)->GetBulletBody());
	bodies.erase(std::remove(bodies.begin(), bodies.end(), body));
}

void PhysicsWorld::Update(float deltaTime)
{
	world->stepSimulation(deltaTime, 10);
}