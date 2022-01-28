#include "CollisionListener.h"
#include "RigidBody.h"
#include "SoundManager.h"

#include <iostream>

void CollisionListener::Collide(Physics::IRigidBody* bodyA, Physics::IRigidBody* bodyB)
{
	RigidBody* bodyACast = dynamic_cast<RigidBody*>(bodyA);
	RigidBody* bodyBCast = dynamic_cast<RigidBody*>(bodyB);
	if ((bodyACast && bodyBCast))
	{
		if (bodyACast->GetShape()->GetShapeType() == Physics::ShapeType::Sphere && bodyBCast->GetShape()->GetShapeType() == Physics::ShapeType::Sphere)
		{
			PlaySoundInfo info;
			SoundManager::PlaySound("ballCollide", info)->setPaused(false);
		}
	}
}