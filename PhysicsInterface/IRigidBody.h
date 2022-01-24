#pragma once

#include <glm/glm.hpp>

namespace Physics
{
	struct RigidBodyInfo
	{
		RigidBodyInfo()
			: mass(1.0f),
			isStatic(false),
			position(glm::vec3(0.0f)),
			velocity(glm::vec3(0.0f)),
			restitution(0.8f),
			friction(0.95f) {}

		bool isStatic;

		glm::vec3 position;
		glm::vec3 velocity;

		float mass;
		float damping;
		float restitution;
		float friction;
	};

	class IRigidBody
	{
	public:
		virtual ~IRigidBody() = default;

		virtual void GetPosition(glm::vec3& pos) = 0;
		virtual glm::vec3 GetPosition() = 0;
		virtual void SetPosition(const glm::vec3& pos) = 0;

		virtual void GetOrientation(glm::quat& orien) = 0;
		virtual void SetOrientation(const glm::quat& orien) = 0;

		virtual void ApplyForce(const glm::vec3& force) = 0;

	protected:
		IRigidBody() = default;

	private:
		IRigidBody(const IRigidBody& other) {}
		IRigidBody& operator=(const IRigidBody& other) { return *this; }
	};
}