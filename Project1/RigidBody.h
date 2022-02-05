#pragma once

#include <IShape.h>
#include <IRigidBody.h>

#include <glm/gtx/quaternion.hpp>

class RigidBody : public Physics::IRigidBody
{
public:
	virtual ~RigidBody();

	// Inherited from IRigidBody
	virtual void GetLinearVelocity(glm::vec3& vel) override { vel = linearVelocity; }
	virtual glm::vec3 GetLinearVelocity() override { return linearVelocity; }
	virtual void SetLinearVelocity(const glm::vec3& vel) override { linearVelocity = vel; }
	virtual void GetPosition(glm::vec3& pos) override { pos = position; }
	virtual glm::vec3 GetPosition() override { return position; }
	virtual void SetPosition(const glm::vec3& pos) override { position = pos; }
	virtual glm::quat GetOrientation() const override { return rotation; }
	virtual void GetOrientation(glm::quat& orien) override { orien = rotation; }
	virtual void SetOrientation(const glm::quat& orien) override { rotation = orien; }
	virtual void ApplyForce(const glm::vec3& force) override { this->force += force; }
	virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) override;
	virtual void ApplyImpulse(const glm::vec3& impulse) override { linearVelocity += impulse * inverseMass; }
	virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) override { ApplyTorqueImpulse(glm::cross(relativePoint, impulse)); }
	virtual void ApplyTorque(const glm::vec3& torque) override { this->torque += torque; }
	virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) override { angularVelocity += torqueImpulse; }
	virtual void SetGravityAcceleration(const glm::vec3& gravity) override { this->gravity = gravity; }
	virtual void UseLocalGravity(bool value) override { useLocalGravity = value; }
	virtual bool IsUseLocalGravity() const override { return useLocalGravity; }

	void UpdateAcceleration();

	void ClearForces();
	void ApplyDamping(float deltaTime);

	// Verlet Steps (integrates Newton's equations of motion)
	void VerletStep1(float deltaTime);
	void VerletStep2(float deltaTime);
	void VerletStep3(float deltaTime);

	bool IsStatic() const { return isStatic; }
	Physics::IShape* GetShape() { return shape; }

private:
	friend class PhysicsFactory;
	friend class CollisionHandler;

	RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape);
	RigidBody(const RigidBody& other) {}
	RigidBody& operator=(const RigidBody& other) { return *this; }

	bool isStatic;

	float mass;
	float inverseMass;

	glm::vec3 linearAcceleration;
	glm::vec3 angularAcceleration;

	glm::vec3 position;
	glm::vec3 previousPosition;

	glm::vec3 linearVelocity;
	glm::vec3 force;

	glm::quat rotation;
	glm::vec3 angularVelocity;
	glm::vec3 torque;

	glm::vec3 gravity;

	Physics::IShape* shape;

	float restitution;
	float friction;

	float linearDamping;
	float angularDamping;

	bool useLocalGravity;
};