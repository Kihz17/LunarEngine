#include "RigidBody.h"


RigidBody::RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape)
	: IRigidBody(),
	shape(shape),
	mass(info.mass),
	isStatic(info.isStatic),
	position(info.position),
	previousPosition(info.position),
	velocity(info.velocity),
	damping(info.damping),
	restitution(info.restitution)
{
	if (isStatic || mass <= 0.0f) // Static object
	{
		inverseMass = 0.0f;
		isStatic = true;
	}
	else // Not static
	{
		inverseMass = 1.0f / mass;
	}
}

RigidBody::~RigidBody()
{

}

void RigidBody::UpdateAcceleration()
{
	if (isStatic) return;

	acceleration = force * inverseMass * localGravity;
}

void RigidBody::ApplyDamping(float deltaTime)
{
	velocity *= pow(1.0f - damping, deltaTime);

	if (glm::length(velocity) < 0.001f)
	{
		velocity = glm::vec3(0.0f);
	}
}

void RigidBody::VerletStep1(float deltaTime)
{
	if (isStatic) return;

	previousPosition = position;
	position += (velocity + acceleration * (deltaTime * 0.5f)) * deltaTime;
}

void RigidBody::VerletStep2(float deltaTime)
{
	if (isStatic) return;

	velocity += acceleration * (deltaTime * 0.5f);
}

void RigidBody::VerletStep3(float deltaTime)
{
	VerletStep2(deltaTime);
}