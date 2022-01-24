#pragma once

#include <IShape.h>
#include <IRigidBody.h>

class RigidBody : public Physics::IRigidBody
{
public:
	RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape);
	virtual ~RigidBody();

	// Inherited from IRigidBody
	virtual void GetPosition(glm::vec3& pos) override { pos = position; }
	virtual glm::vec3 GetPosition() override { return position; }
	virtual void SetPosition(const glm::vec3& pos) override { position = pos; }
	virtual void GetOrientation(glm::quat& orien) override {} // TODO:
	virtual void SetOrientation(const glm::quat& orien) override {}  // TODO:
	virtual void ApplyForce(const glm::vec3& force) override { this->force += force; }

	void SetGravityAcceleration(const glm::vec3& gravity) { localGravity = gravity; }
	void UpdateAcceleration();

	void ClearForces() { force = glm::vec3(0.0f); }
	void ApplyDamping(float deltaTime);

	// Verlet Steps (integrates Newton's equations of motion)
	void VerletStep1(float deltaTime);
	void VerletStep2(float deltaTime);
	void VerletStep3(float deltaTime);

	bool IsStatic() const { return isStatic; }
	Physics::IShape* GetShape() { return shape; }

private:
	friend class CollisionHandler;

	RigidBody(const RigidBody& other) {}
	RigidBody& operator=(const RigidBody& other) { return *this; }

	bool isStatic;

	float mass;
	float inverseMass;

	glm::vec3 position;
	glm::vec3 previousPosition;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 force;
	glm::vec3 localGravity;

	Physics::IShape* shape;

	float damping;
	float restitution;
	float friction;
};