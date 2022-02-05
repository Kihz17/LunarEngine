#include "RigidBody.h"

#include <iostream>

RigidBody::RigidBody(const Physics::RigidBodyInfo info, Physics::IShape* shape)
	: IRigidBody(),
	shape(shape),
	mass(info.mass),
	isStatic(info.isStatic),
	position(info.position),
	linearVelocity(info.linearVelocity),
	rotation(info.rotation),
	angularVelocity(info.angularVelocity),
	linearDamping(info.linearDamping),
	angularDamping(info.angularDamping),
	friction(info.friction),
	restitution(info.restitution),
	useLocalGravity(false)
{
	if (isStatic || mass <= 0.0f) // Static object
	{
		mass = 0.0f;
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
	delete shape;
}

void RigidBody::ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint)
{
	ApplyForce(force);
	ApplyTorque(glm::cross(relativePoint, force));
}

void RigidBody::UpdateAcceleration()
{
	if (isStatic) return;

	linearAcceleration = force * inverseMass + gravity;
	angularAcceleration = torque;
}

void RigidBody::ClearForces()
{
	force = glm::vec3(0.0f);
	torque = glm::vec3(0.0f);
}

void RigidBody::ApplyDamping(float deltaTime)
{
	linearVelocity *= pow(1.0f - linearDamping, deltaTime);
	angularVelocity *= pow(1.0f - angularDamping, deltaTime);

	if (glm::length(linearVelocity) < 0.001f)
	{
		linearVelocity = glm::vec3(0.0f);
	}
}

void RigidBody::VerletStep1(float deltaTime)
{
	if (isStatic) return;

	previousPosition = position;
	position += (linearVelocity + linearAcceleration * (deltaTime * 0.5f)) * deltaTime;

	glm::vec3 axis = (angularVelocity + angularAcceleration * (deltaTime * 0.5f)) * deltaTime;
	float angle = glm::length(axis);
	axis = glm::normalize(axis);
	if (angle != 0.0f)
	{
		glm::quat rot = glm::angleAxis(angle, axis);
		rotation *= rot;
	}
}

void RigidBody::VerletStep2(float deltaTime)
{
	if (isStatic) return;

	linearVelocity += linearAcceleration * (deltaTime * 0.5f);
	angularVelocity += angularAcceleration * (deltaTime * 0.5f);
}

void RigidBody::VerletStep3(float deltaTime)
{
	VerletStep2(deltaTime);
}