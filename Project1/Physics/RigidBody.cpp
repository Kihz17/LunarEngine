#include "RigidBody.h"

#include <iostream>

RigidBody::RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape)
	: IRigidBody()
{
	btQuaternion orientation = BulletUtils::GLMQuatToBullet(info.rotation);
	btVector3 position = BulletUtils::GLMVec3ToBullet(info.position);

	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(orientation, position));
	btCollisionShape* bulletShape = BulletUtils::ToBulletShape(shape);

	btVector3 inertia(0.0f, 0.0f, 0.0f);
	if (info.mass != 0.0f) bulletShape->calculateLocalInertia(info.mass, inertia);

	btRigidBody::btRigidBodyConstructionInfo ci(info.mass, motionState, bulletShape, inertia);
	bulletBody = new btRigidBody(ci);
	bulletBody->setActivationState(DISABLE_DEACTIVATION);
}

RigidBody::~RigidBody()
{
	delete rigidbody;
}

bool RigidBody::IsStatic() const
{
	return bulletBody->isStaticObject();
}

void RigidBody::GetPosition(glm::vec3& positionOut)
{
	positionOut = BulletUtils::BulletVec3ToGLM(bulletBody->getCenterOfMassPosition());
}

glm::vec3 RigidBody::GetPosition() const
{
	return BulletUtils::BulletVec3ToGLM(bulletBody->getCenterOfMassPosition());
}

void RigidBody::SetPosition(const glm::vec3& positionIn)
{
	// Can't do this.
	assert(0);
}

void RigidBody::GetOrientation(glm::quat& orientationOut)
{
	orientationOut = BulletUtils::BulletQuatToGLM(bulletBody->getOrientation());
}

glm::quat RigidBody::GetOrientation() const
{
	return BulletUtils::BulletQuatToGLM(bulletBody->getOrientation());
}

void RigidBody::SetOrientation(const glm::quat& orientationIn)
{
	// Can't do this.
	assert(0);
}

void RigidBody::ApplyForce(const glm::vec3& force)
{
	btVector3 btForce = BulletUtils::GLMVec3ToBullet(force);
	bulletBody->applyCentralForce(btForce);
}

void RigidBody::ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint)
{
	btVector3 btForce = BulletUtils::GLMVec3ToBullet(force);
	btVector3 btForceAtPoint = BulletUtils::GLMVec3ToBullet(relativePoint);

	bulletBody->applyForce(btForce, btForceAtPoint);
}

void RigidBody::ApplyImpulse(const glm::vec3& impulse)
{
	btVector3 btImpulse = BulletUtils::GLMVec3ToBullet(impulse);
	bulletBody->applyCentralImpulse(btImpulse);
}

void RigidBody::ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint)
{
	btVector3 btImpulse = BulletUtils::GLMVec3ToBullet(impulse);
	btVector3 btImpulseAtPoint = BulletUtils::GLMVec3ToBullet(relativePoint);

	bulletBody->applyImpulse(btImpulse, btImpulseAtPoint);
}

void RigidBody::ApplyTorque(const glm::vec3& torque)
{
	btVector3 btTorque = BulletUtils::GLMVec3ToBullet(torque);
	bulletBody->applyTorque(btTorque);
}

void RigidBody::ApplyTorqueImpulse(const glm::vec3& torqueImpulse)
{
	btVector3 btTorqueImpulse = BulletUtils::GLMVec3ToBullet(torqueImpulse);
	bulletBody->applyTorque(btTorqueImpulse);
}

glm::vec3 RigidBody::GetLinearVelocity() const
{
	return BulletUtils::BulletVec3ToGLM(bulletBody->getLinearVelocity());
}

Physics::IShape* RigidBody::GetShape()
{
	return nullptr;
}