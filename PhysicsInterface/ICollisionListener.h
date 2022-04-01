#pragma once

#include "IPhysicsWorld.h"
#include "ICollisionBody.h"

namespace Physics
{
	class ICollisionListener
	{
	public:
		virtual ~ICollisionListener() {}

		virtual void Collide(ICollisionBody& bodyA, ICollisionBody& bodyB) = 0;

	protected:
		ICollisionListener() = default;

	private:
		ICollisionListener(const ICollisionListener& other) { }
		ICollisionListener& operator=(const ICollisionListener& other) { return *this; }
	};
}