#pragma once

#include "IPhysicsWorld.h"
#include "IRigidBody.h"
#include "IShape.h"

namespace Physics
{
	class IPhysicsFactory
	{
	public:
		virtual ~IPhysicsFactory() {}

		virtual IPhysicsWorld* CreateWorld() = 0;

		virtual IRigidBody* CreateRigidBody(const RigidBodyInfo& desc, IShape* shape) = 0;

	protected:
		IPhysicsFactory() {}

	private:
		IPhysicsFactory(const IPhysicsFactory&) {}
		IPhysicsFactory& operator=(const IPhysicsFactory&) { return *this; }
	};
}
