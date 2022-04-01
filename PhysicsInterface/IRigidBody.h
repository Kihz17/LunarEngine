#pragma once

#include "ICollisionBody.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Physics
{
	struct RigidBodyInfo
	{
		float mass = 1.0f;
		bool isStatic = false;

		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 linearVelocity = glm::vec3(0.0f);

		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 angularVelocity = glm::vec3(0.0f);

		float linearDamping = 0.0f;
		float angularDamping = 0.0f;
		float restitution = 0.8f;
		float friction = 0.95f;
	};

	class IRigidBody : public ICollisionBody
	{
	public:
		virtual ~IRigidBody() {}

		virtual void GetPosition(glm::vec3& positionOut) = 0;
		virtual void SetPosition(const glm::vec3& positionIn) = 0;

		virtual void GetOrientation(glm::quat& orientationOut) = 0;
		virtual void SetOrientation(const glm::quat& orientationIn) = 0;

		virtual void ApplyForce(const glm::vec3& force) = 0;
		virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) = 0;

		virtual void ApplyImpulse(const glm::vec3& impulse) = 0;
		virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) = 0;

		virtual void ApplyTorque(const glm::vec3& torque) = 0;
		virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) = 0;

	protected:
		IRigidBody() : ICollisionBody(CollisionBodyType::Rigid) { }

	private:
		IRigidBody(const IRigidBody&) : ICollisionBody(CollisionBodyType::Rigid) { }
		IRigidBody& operator=(const IRigidBody&) { return *this; }
	};
}