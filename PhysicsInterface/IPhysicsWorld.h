#pragma once

#include "ICollisionListener.h"

#include <glm/glm.hpp>

namespace Physics
{
	class IPhysicsWorld
	{
	public:
		virtual ~IPhysicsWorld() = default;

		virtual void SetGravity(const glm::vec3& gravity) = 0;
		virtual void RegisterCollisionListener(ICollisionListener* listener) = 0;

		virtual void AddBody(ICollisionBody* body) = 0;
		virtual void RemoveBody(ICollisionBody* body) = 0;

		virtual void Update(float deltaTime) = 0;

	protected:
		IPhysicsWorld() = default;

	private:
		IPhysicsWorld(const IPhysicsWorld& other) { }
		IPhysicsWorld& operator=(const IPhysicsWorld& other) { return *this; }
	};
}