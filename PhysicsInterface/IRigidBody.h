#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Physics
{
	struct RigidBodyInfo
	{
		RigidBodyInfo()
			: mass(1.0f),
			isStatic(false),
			position(glm::vec3(0.0f)),
			linearVelocity(glm::vec3(0.0f)),
			restitution(0.8f),
			friction(0.95f),
			rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			angularVelocity(glm::vec3(0.0f)),
			linearDamping(0.001f),
			angularDamping(0.001f)
		{}

		bool isStatic;

		glm::vec3 position;
		glm::vec3 linearVelocity;

		glm::quat rotation;
		glm::vec3 angularVelocity;

		float mass;
		float linearDamping;
		float angularDamping;
		float restitution;
		float friction;
	};

	class IRigidBody
	{
	public:
		virtual ~IRigidBody() = default;

		virtual void GetLinearVelocity(glm::vec3& vel) = 0;
		virtual glm::vec3 GetLinearVelocity() = 0;
		virtual void SetLinearVelocity(const glm::vec3& vel) = 0;

		virtual void GetPosition(glm::vec3& pos) = 0;
		virtual glm::vec3 GetPosition() = 0;
		virtual void SetPosition(const glm::vec3& pos) = 0;

		virtual glm::quat GetOrientation() const = 0;
		virtual void GetOrientation(glm::quat& orien) = 0;
		virtual void SetOrientation(const glm::quat& orien) = 0;

		virtual void ApplyForce(const glm::vec3& force) = 0;
		virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) = 0;

		virtual void ApplyImpulse(const glm::vec3& impulse) = 0;
		virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) = 0;

		virtual void ApplyTorque(const glm::vec3& torque) = 0;
		virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) = 0;

		virtual void SetGravityAcceleration(const glm::vec3& gravity) = 0;
		virtual void UseLocalGravity(bool value) = 0;
		virtual bool IsUseLocalGravity() const = 0;

	protected:
		IRigidBody() = default;

	private:
		IRigidBody(const IRigidBody& other) {}
		IRigidBody& operator=(const IRigidBody& other) { return *this; }
	};
}