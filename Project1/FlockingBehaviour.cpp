#include "FlockingBehaviour.h"

FlockingBehaviour::FlockingBehaviour(Physics::IRigidBody* rigidBody, const FlockingInfo& info, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	group(info.group),
	separationRadius(info.separationRadius),
	alignmentRadius(info.alignmentRadius),
	cohesionRadius(info.cohesionRadius),
	separationWeight(info.separationWeight),
	alignmentWeight(info.alignmentWeight),
	cohesionWeight(info.cohesionWeight),
	moveDirection(0.0f)
{

}

FlockingBehaviour::~FlockingBehaviour()
{

}

glm::vec3 FlockingBehaviour::ComputeSteeringForce()
{
	glm::vec3 steeringForce = moveDirection;
	steeringForce += Separate() * separationWeight;
	steeringForce += Align() * alignmentWeight;
	steeringForce += Cohesion() * cohesionWeight;
	return steeringForce;
}

glm::vec3 FlockingBehaviour::Separate()
{
	glm::vec3 force(0.0f);
	int neighbourhoodCount = 0;
	for (RigidBodyComponent* rb : group->rigidbodies)
	{
		float distance = glm::length(rigidBody->GetPosition() - rb->ptr->GetPosition());
		if (distance < separationRadius && distance > 0.0f)
		{
			glm::vec3 fleeVec = glm::normalize(rigidBody->GetPosition() - rb->ptr->GetPosition());
			fleeVec /= distance;
			force += fleeVec;
			neighbourhoodCount++;
		}
	}

	glm::vec3 separationForce(0.0f);
	if (neighbourhoodCount > 0)
	{
		glm::vec3 desiredVelocity = glm::normalize(force / (float)neighbourhoodCount) * speed;
		separationForce = desiredVelocity - rigidBody->GetLinearVelocity();
		if (glm::length(separationForce) > maxForce)
		{
			separationForce = glm::normalize(separationForce) * maxForce;
		}
	}

	return separationForce;
}

glm::vec3 FlockingBehaviour::Align()
{
	glm::vec3 force(0.0f);
	int neighbourhoodCount = 0;
	for (RigidBodyComponent* rb : group->rigidbodies)
	{
		float distance = glm::length(rigidBody->GetPosition() - rb->ptr->GetPosition());
		if (distance < alignmentRadius && distance > 0.0f)
		{
			force += rb->ptr->GetLinearVelocity();
			neighbourhoodCount++;
		}
	}

	glm::vec3 alignmentForce(0.0f);
	if (neighbourhoodCount > 0)
	{
		glm::vec3 desiredVelocity = glm::normalize(force / (float)neighbourhoodCount) * speed;
		alignmentForce = desiredVelocity - rigidBody->GetLinearVelocity();
		if (glm::length(alignmentForce) > maxForce)
		{
			alignmentForce = glm::normalize(alignmentForce) * maxForce;
		}
	}

	return alignmentForce;
}

glm::vec3 FlockingBehaviour::Cohesion()
{
	glm::vec3 pos(0.0f);
	int neighbourhoodCount = 0;
	for (RigidBodyComponent* rb : group->rigidbodies)
	{
		float distance = glm::length(rigidBody->GetPosition() - rb->ptr->GetPosition());
		if (distance < cohesionRadius && distance > 0.0f)
		{
			pos += rb->ptr->GetPosition();
			neighbourhoodCount++;
		}
	}

	glm::vec3 cohesionForce(0.0f);
	if (neighbourhoodCount > 0)
	{
		glm::vec3 targetPos = pos / (float) neighbourhoodCount;
		glm::vec3 desiredVelocity = glm::normalize(targetPos - rigidBody->GetPosition()) * speed;
		cohesionForce = desiredVelocity - rigidBody->GetLinearVelocity();
		if (glm::length(cohesionForce) > maxForce)
		{
			cohesionForce = glm::normalize(cohesionForce) * maxForce;
		}
	}

	return cohesionForce;
}