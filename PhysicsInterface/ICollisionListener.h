#pragma once

#include "IPhysicsWorld.h"
#include "IRigidBody.h"

namespace Physics
{
	enum class CollisionType
	{
		SphereSphere,
		SpherePlane,
		SphereAABB
	};
		
	template<class RigidBodyOwner> class ICollisionListener
	{
	public:
		struct CollisionEvent
		{
			CollisionType type;
			IRigidBody* bodyA;
			IRigidBody* bodyB;
			IPhysicsWorld<RigidBodyOwner>* physicsWorld;

			bool isCancelled = false;
		};

		virtual ~ICollisionListener() = default;

		virtual void Collide(CollisionEvent& collisionEvent) = 0;

	protected:
		ICollisionListener() = default;

	private:
		ICollisionListener(const ICollisionListener& other) { }
		ICollisionListener& operator=(const ICollisionListener& other) { return *this; }
	};
}