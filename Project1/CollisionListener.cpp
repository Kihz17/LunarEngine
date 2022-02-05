#include "CollisionListener.h"
#include "RigidBody.h"
#include "SoundManager.h"
#include "Entity.h"
#include "Components.h"

#include <iostream>

CollisionListener::CollisionListener(EntityManager& entityManager)
	: entityManager(entityManager)
{

}

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

		// Player collided with an entity, they should die
		if (tagA->HasTag("player") && (tagB->HasTag("enemy") || tagB->HasTag("bullet")))
		{
			// TODO: Kill player
		}
		else if (tagB->HasTag("player") && (tagA->HasTag("enemy") || tagA->HasTag("bullet")))
		{
			// TODO: Kill player
		}
	}

	// Remove bullets on collision
	if (tagA && tagA->HasTag("bullet"))
	{
		collisionEvent.physicsWorld->RemoveRigidBody(collisionEvent.bodyA);
		entityManager.DeleteEntity(ownerA);
	}
	if (tagB && tagB->HasTag("bullet"))
	{
		collisionEvent.physicsWorld->RemoveRigidBody(collisionEvent.bodyB);
		entityManager.DeleteEntity(ownerB);
	}
}