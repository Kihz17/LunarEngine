#pragma once

#include "BulletUtils.h"

#include <IShape.h>
#include <IRigidBody.h>

#include <glm/gtx/quaternion.hpp>

#include <Bullet/btBulletDynamicsCommon.h>

class RigidBody : public Physics::IRigidBody
{
public:
	RigidBody(const Physics::RigidBodyInfo info, btCollisionShape* shape);
	virtual ~RigidBody();

	// Inherited from IRigidBody
	virtual void GetLinearVelocity(glm::vec3& vel) override;
	virtual glm::vec3 GetLinearVelocity() override;
	virtual void SetLinearVelocity(const glm::vec3& vel) override;
	virtual void GetPosition(glm::vec3& pos) override;
	virtual glm::vec3 GetPosition() override;
	virtual void SetPosition(const glm::vec3& pos) override;
	virtual glm::quat GetOrientation() const override;
	virtual void GetOrientation(glm::quat& orien) override;
	virtual void SetOrientation(const glm::quat& orien) override;
	virtual void ApplyForce(const glm::vec3& force) override;
	virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) override;
	virtual void ApplyImpulse(const glm::vec3& impulse) override;
	virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) override;
	virtual void ApplyTorque(const glm::vec3& torque) override;
	virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) override;
	virtual void SetGravityAcceleration(const glm::vec3& gravity) override;

	bool IsStatic() const { return rigidbody->isStaticObject(); }
	btCollisionShape* GetShape() { return rigidbody->getCollisionShape(); }
	btRigidBody* GetBulletBody() { return rigidbody; }

private:
	friend class PhysicsFactory;
	friend class CollisionHandler;

	RigidBody(const RigidBody& other) {}
	RigidBody& operator=(const RigidBody& other) { return *this; }

	btRigidBody* rigidbody;
};