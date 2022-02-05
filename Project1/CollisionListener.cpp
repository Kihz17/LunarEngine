#include "CollisionListener.h"
#include "RigidBody.h"
#include "SoundManager.h"
#include "Entity.h"
#include "Components.h"

#include <iostream>

void CollisionListener::Collide(CollisionEvent& collisionEvent)
{
	Entity* ownerA = collisionEvent.physicsWorld->GetRigidBodyOwner(collisionEvent.bodyA);
	Entity* ownerB = collisionEvent.physicsWorld->GetRigidBodyOwner(collisionEvent.bodyB);
	TagComponent* tagA = ownerA->GetComponent<TagComponent>();
	TagComponent* tagB = ownerB->GetComponent<TagComponent>();
	if (tagA && tagB)
	{
		if (tagA->HasTag("bullet") && tagB->HasTag("bullet")) // Bullets should not collide
		{
			collisionEvent.isCancelled = true;
			return;
		}
	}

	if (tagA && tagA->HasTag("bullet"))
	{
		std::cout << "Bullet remove!\n";
		collisionEvent.physicsWorld->RemoveRigidBody(collisionEvent.bodyA);
		delete ownerA;
		//delete collisionEvent.ownerA;
		// TODO: Remove entity
		// TODO: Remove rigid from phys world
	}
	if (tagB && tagB->HasTag("bullet"))
	{
		std::cout << "Bullet remove!\n";
		collisionEvent.physicsWorld->RemoveRigidBody(collisionEvent.bodyB);
		delete ownerB;
		//delete collisionEvent.ownerB;
		// TODO: Remove entity
	}
}