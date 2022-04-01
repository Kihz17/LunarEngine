#pragma once

#include "BulletUtils.h"

#include <IShape.h>
#include <IRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/btBulletCollisionCommon.h>

class RigidBody : public Physics::IRigidBody
{
public:
	RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape);
	virtual ~RigidBody();

	virtual void GetPosition(glm::vec3& positionOut) override;
	virtual void SetPosition(const glm::vec3& positionIn) override;

	virtual void GetOrientation(glm::quat& orientationOut) override;
	virtual void SetOrientation(const glm::quat& orientationIn) override;

	virtual void ApplyForce(const glm::vec3& force) override;
	virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) override;

	virtual void ApplyImpulse(const glm::vec3& impulse) override;
	virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) override;

	virtual void ApplyTorque(const glm::vec3& torque) override;
	virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) override;

	Physics::IShape* GetShape();
	bool IsStatic() const;

	btRigidBody* GetBulletBody() { return bulletBody; }

private:
	friend class PhysicsFactory;
	friend class CollisionHandler;

	btRigidBody* bulletBody;

	RigidBody(const RigidBody& other) {}
	RigidBody& operator=(const RigidBody& other) { return *this; }

	btRigidBody* rigidbody;
};