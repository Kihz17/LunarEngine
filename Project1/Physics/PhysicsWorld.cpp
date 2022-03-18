#include "PhysicsWorld.h"

#include <iostream>

PhysicsWorld::PhysicsWorld()
	: IPhysicsWorld(),
	collisionHandler(new CollisionHandler())
{

}

PhysicsWorld::~PhysicsWorld()
{
	for (unsigned int i = 0; i < rigidBodies.size(); i++) delete rigidBodies[i];
	rigidBodies.clear();
	delete collisionHandler;
}

void PhysicsWorld::AddRigidBody(Physics::IRigidBody* body, Entity* owner)
{
	if (!body) return;

	RigidBody* rb = dynamic_cast<RigidBody*>(body);
	rigidBodiesToOwner.insert({rb , owner });
	rigidBodies.push_back(rb);
}

void PhysicsWorld::RemoveRigidBody(Physics::IRigidBody* body)
{
	RigidBody* rigidBody = dynamic_cast<RigidBody*>(body);
	rigidBodiesToRemove.push_back(dynamic_cast<RigidBody*>(body)); // Flag to remove at the end of update
}

Entity* PhysicsWorld::GetRigidBodyOwner(Physics::IRigidBody* body)
{
	std::unordered_map<RigidBody*, Entity*>::iterator it = rigidBodiesToOwner.find(dynamic_cast<RigidBody*>(body));
	if (it == rigidBodiesToOwner.end()) return nullptr;
	return it->second;
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
			if(!body->IsUseLocalGravity()) body->SetGravityAcceleration(gravity);
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
	collisionHandler->Collide(deltaTime, rigidBodies, collidingBodies, collisionListeners, this);

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

	// TODO: Add another "type" of listener that will be called after the collision happens?
	// For each colliding pair, tell the listener that they have collided
	/*for (int i = 0; i < collidingBodies.size(); i++)
	{
		collisionListener->Collide(collidingBodies[i].bodyA, collidingBodies[i].bodyB);
	}*/

	for (RigidBody* rb : rigidBodiesToRemove)
	{
		// Remove from vector
		int removeIndex = -1;
		for (int i = 0; i < rigidBodies.size(); i++)
		{
			if (rigidBodies[i] == rb)
			{
				removeIndex = i;
				break;
			}
		}
		
		if (removeIndex != -1) rigidBodies.erase(rigidBodies.begin() + removeIndex);
		rigidBodiesToOwner.erase(rb);
	}
}