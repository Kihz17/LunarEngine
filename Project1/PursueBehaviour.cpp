#include "PursueBehaviour.h"
#include "Components.h"

#include <iostream>

PursueBehaviour::PursueBehaviour(Physics::IRigidBody* rigidBody, float maxSteps, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	maxSteps(maxSteps)
{

}

PursueBehaviour::~PursueBehaviour()
{

}

glm::vec3 PursueBehaviour::ComputeSteeringForce()
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	RigidBodyComponent* rigidComp = target->GetComponent<RigidBodyComponent>();
	if (!rigidComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = rigidComp->ptr->GetPosition() - rigidBody->GetPosition();
	float distance = glm::length(direction);

	float speed = glm::length(rigidBody->GetLinearVelocity());
	float t;
	if (speed <= distance / maxSteps) // Target is far away
	{
		t = maxSteps;
	}
	else // Target is close
	{
		t = distance / speed;
	}

	glm::vec3 futurePos = rigidComp->ptr->GetPosition() + rigidComp->ptr->GetLinearVelocity() * t;
	glm::vec3 velocity = glm::normalize(futurePos - rigidBody->GetPosition()) * this->speed;
	//std::cout << "Vel: " << velocity.x << " " << velocity.y << " " << velocity.z << " " << "\n";

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);

	return steer;
}