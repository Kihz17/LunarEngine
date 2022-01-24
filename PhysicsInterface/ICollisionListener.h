#pragma once

#include "IRigidBody.h"
namespace Physics
{
	class ICollisionListener
	{
	public:
		virtual ~ICollisionListener() = default;

		virtual void Collide(IRigidBody* bodyA, IRigidBody* bodyB) = 0;

	protected:
		ICollisionListener() = default;

	private:
		ICollisionListener(const ICollisionListener& other) { }
		ICollisionListener& operator=(const ICollisionListener& other) { return *this; }
	};
}