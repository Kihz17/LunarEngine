#pragma once

#include "IPhysicsWorld.h"
#include "IShape.h"

namespace Physics
{
	template<class RigidBodyOwner> class IPhysicsFactory
	{
	public:
		virtual ~IPhysicsFactory() = default;

		virtual IPhysicsWorld<RigidBodyOwner>* CreateWorld() = 0;

		virtual IRigidBody* CreateRigidBody(const RigidBodyInfo& info, IShape* shape) = 0;

	protected:
		IPhysicsFactory() = default;

	private:
		IPhysicsFactory(const IPhysicsFactory& other) {}
		IPhysicsFactory& operator=(const IPhysicsFactory& other) { return *this; }
	};
}