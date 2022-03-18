#include "CollisionListener.h"
#include "RigidBody.h"
#include "SoundManager.h"
#include "Entity.h"
#include "Components.h"

#include <iostream>

constexpr float bulletDamage = 10.0f;

CollisionListener::CollisionListener(EntityManager& entityManager)
	: entityManager(entityManager)
{

}

void CollisionListener::Collide(CollisionEvent& collisionEvent)
{
	Entity* ownerA = collisionEvent.physicsWorld->GetRigidBodyOwner(collisionEvent.bodyA);
	Entity* ownerB = collisionEvent.physicsWorld->GetRigidBodyOwner(collisionEvent.bodyB);
	if (!ownerA->IsValid() || !ownerB->IsValid()) return;
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
			ResetGame(collisionEvent.physicsWorld);
		}
		else if (tagB->HasTag("player") && (tagA->HasTag("enemy") || tagA->HasTag("bullet")))
		{
			ResetGame(collisionEvent.physicsWorld);
		}
		else if (tagB->HasTag("enemy") && tagA->HasTag("bullet"))
		{
			if (tagB->HasTag("health"))
			{
				TagValue<float>* health = tagB->GetValue<float>("health");
				health->value -= bulletDamage;
				if(health->value <= 0.0f) RemoveEntity(collisionEvent.bodyB, ownerB, collisionEvent.physicsWorld);
			}
			else
			{
				RemoveEntity(collisionEvent.bodyB, ownerB, collisionEvent.physicsWorld);
			}
			
			RemoveEntity(collisionEvent.bodyA, ownerA, collisionEvent.physicsWorld);
		}
		else if (tagA->HasTag("enemy") && tagB->HasTag("bullet"))
		{
			if (tagA->HasTag("health"))
			{
				TagValue<float>* health = tagA->GetValue<float>("health");
				health->value -= bulletDamage;
				if (health->value <= 0.0f) RemoveEntity(collisionEvent.bodyA, ownerA, collisionEvent.physicsWorld);
			}
			else
			{
				RemoveEntity(collisionEvent.bodyA, ownerA, collisionEvent.physicsWorld);
			}

			RemoveEntity(collisionEvent.bodyB, ownerB, collisionEvent.physicsWorld);
		}
	}
	else if (tagA && !tagB && tagA->HasTag("bullet"))
	{
		RemoveEntity(collisionEvent.bodyA, ownerA, collisionEvent.physicsWorld);
	}
	else if (tagB && !tagA && tagB->HasTag("bullet"))
	{
		RemoveEntity(collisionEvent.bodyB, ownerB, collisionEvent.physicsWorld);
	}
}

void CollisionListener::ResetGame(Physics::IPhysicsWorld<Entity>* physicsWorld)
{
	std::vector<Entity*> entities = entityManager.GetEntities();
	for (Entity* entity : entities)
	{
		TagComponent* tagComp = entity->GetComponent<TagComponent>();
		if (!tagComp) continue;

		if (tagComp->HasTag("player"))
		{
			entity->GetComponent<RigidBodyComponent>()->ptr->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (tagComp->HasTag("enemy") || tagComp->HasTag("bullet"))
		{
			RemoveEntity(entity->GetComponent<RigidBodyComponent>()->ptr, entity, physicsWorld);
		}
	}
}

void CollisionListener::RemoveEntity(Physics::IRigidBody* body, Entity* entity, Physics::IPhysicsWorld<Entity>* physicsWorld)
{
	physicsWorld->RemoveRigidBody(body);
	entityManager.DeleteEntity(entity);
}