#include "RigidBody.h"

#include <iostream>

RigidBody::RigidBody(const Physics::RigidBodyInfo info, btCollisionShape* shape)
	: IRigidBody(),
	rigidbody(new btRigidBody(info.mass, new btDefaultMotionState(BulletUtils::GLMMat4ToBullet(info.initialTransform)), shape, BulletUtils::GLMVec3ToBullet(info.intertia)))
{

}

RigidBody::~RigidBody()
{
	delete rigidbody;
}

void RigidBody::GetLinearVelocity(glm::vec3& vel) 
{
	vel = BulletUtils::BulletVec3ToGLM(rigidbody->getLinearVelocity());
}

glm::vec3 RigidBody::GetLinearVelocity() 
{ 
	return BulletUtils::BulletVec3ToGLM(rigidbody->getLinearVelocity()); 
}

void RigidBody::SetLinearVelocity(const glm::vec3& vel) 
{ 
	rigidbody->setLinearVelocity(BulletUtils::GLMVec3ToBullet(vel)); 
}

void RigidBody::GetPosition(glm::vec3& pos) 
{ 
	pos = BulletUtils::BulletVec3ToGLM(rigidbody->getCenterOfMassPosition()); 
}

glm::vec3 RigidBody::GetPosition() 
{ 
	return BulletUtils::BulletVec3ToGLM(rigidbody->getCenterOfMassPosition()); 
}

void RigidBody::SetPosition(const glm::vec3& pos)
{
	btTransform transform = rigidbody->getWorldTransform();
	transform.setOrigin(BulletUtils::GLMVec3ToBullet(pos));
	rigidbody->setWorldTransform(transform);
	rigidbody->getMotionState()->setWorldTransform(transform);
}

glm::quat RigidBody::GetOrientation() const
{
	return BulletUtils::BulletQuatToGLM(rigidbody->getOrientation());
}

void RigidBody::GetOrientation(glm::quat& orien)
{
	orien = BulletUtils::BulletQuatToGLM(rigidbody->getOrientation());
}

void RigidBody::SetOrientation(const glm::quat& orien)
{ 
	btTransform transform = rigidbody->getWorldTransform();
	transform.setRotation(BulletUtils::GLMQuatToBullet(orien));
	rigidbody->setWorldTransform(transform);
}

void RigidBody::ApplyForce(const glm::vec3& force)
{ 
	rigidbody->applyForce(BulletUtils::GLMVec3ToBullet(force), btVector3(0.0f, 0.0f, 0.0f));
}

void RigidBody::ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint)
{
	rigidbody->applyForce(BulletUtils::GLMVec3ToBullet(force), BulletUtils::GLMVec3ToBullet(relativePoint));
}

void RigidBody::ApplyImpulse(const glm::vec3& impulse)
{ 
	rigidbody->applyImpulse(BulletUtils::GLMVec3ToBullet(impulse), btVector3(0.0f, 0.0f, 0.0f));
}

void RigidBody::ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint)
{ 
	rigidbody->applyImpulse(BulletUtils::GLMVec3ToBullet(impulse), BulletUtils::GLMVec3ToBullet(relativePoint));
}

void RigidBody::ApplyTorque(const glm::vec3& torque)
{ 
	rigidbody->applyTorque(BulletUtils::GLMVec3ToBullet(torque));
}

void RigidBody::ApplyTorqueImpulse(const glm::vec3& torqueImpulse)
{ 
	rigidbody->applyTorqueImpulse(BulletUtils::GLMVec3ToBullet(torqueImpulse));
}

void RigidBody::SetGravityAcceleration(const glm::vec3& gravity)
{ 
	rigidbody->setGravity(BulletUtils::GLMVec3ToBullet(gravity));
}