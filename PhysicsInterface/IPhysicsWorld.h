#pragma once

#include "IRigidBody.h"

namespace Physics
{
	template <class T> class ICollisionListener;
	template <class RigidBodyOwner> class IPhysicsWorld
	{
	public:
		virtual ~IPhysicsWorld() = default;

		virtual void SetGravity(const glm::vec3& gravity) = 0;
		virtual void RegisterCollisionListener(ICollisionListener<RigidBodyOwner>* listener) = 0;
		virtual void AddRigidBody(IRigidBody* body, RigidBodyOwner* owner) = 0;
		virtual void RemoveRigidBody(IRigidBody* body) = 0;
		virtual RigidBodyOwner* GetRigidBodyOwner(IRigidBody* body) = 0;
		virtual void Update(float deltaTime) = 0;

	protected:
		IPhysicsWorld() = default;

	private:
		IPhysicsWorld(const IPhysicsWorld& other) { }
		IPhysicsWorld& operator=(const IPhysicsWorld& other) { return *this; }
	};
}