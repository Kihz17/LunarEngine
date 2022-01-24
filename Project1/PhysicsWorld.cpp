#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld()
	: IPhysicsWorld(),
	collisionListener(0),
	collisionHandler(new CollisionHandler())
{

}

PhysicsWorld::~PhysicsWorld()
{
	for (unsigned int i = 0; i < rigidBodies.size(); i++) delete rigidBodies[i];
	rigidBodies.clear();
	delete collisionHandler;
}

void PhysicsWorld::AddRigidBody(Physics::IRigidBody* body)
{
	if (!body) return;

	rigidBodies.push_back(dynamic_cast<RigidBody*>(body));
}

void PhysicsWorld::RemoveRigidBody(Physics::IRigidBody* body)
{
	RigidBody* rigidBody = dynamic_cast<RigidBody*>(body);
	std::remove(rigidBodies.begin(), rigidBodies.end(), rigidBody);
}

void PhysicsWorld::Update(float deltaTime)
{
	int rigidBodyCount = rigidBodies.size();

	// Update rigid bodies
	for (int i = 0; i < rigidBodyCount; i++)
	{
		RigidBody* body = rigidBodies[i];
		if (!body->IsStatic()) // We should update!
		{
			body->SetGravityAcceleration(gravity);
			body->UpdateAcceleration();
		}
	}

	// Verlet
	for (int i = 0; i < rigidBodyCount; i++)
	{
		RigidBody* body = rigidBodies[i];
		if (!body->IsStatic()) // We should update!
		{
			body->VerletStep3(deltaTime);
			body->ApplyDamping(deltaTime * 0.5f);
		}
	}

	// Verlet positions
	for (int i = 0; i < rigidBodyCount; i++)
	{
		RigidBody* body = rigidBodies[i];
		if (!body->IsStatic()) // We should update!
		{
			body->VerletStep1(deltaTime);
		}
	}

	// Collision Resolution
	std::vector<CollidingBodies> collidingBodies;
	collisionHandler->Collide(deltaTime, rigidBodies, collidingBodies);

	// Verlet
	for (int i = 0; i < rigidBodyCount; i++)
	{
		RigidBody* body = rigidBodies[i];
		if (!body->IsStatic()) // We should update!
		{
			body->VerletStep2(deltaTime);
			body->ApplyDamping(deltaTime * 0.5f);
			body->ClearForces();
		}
	}

	// For each colliding pair, tell the listener that they have collided
	for (int i = 0; i < collidingBodies.size(); i++)
	{
		collisionListener->Collide(collidingBodies[i].bodyA, collidingBodies[i].bodyB);
	}
}