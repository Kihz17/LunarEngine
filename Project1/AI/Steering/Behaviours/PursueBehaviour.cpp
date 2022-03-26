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
	//if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	//RigidBodyComponent* rigidComp = target->GetComponent<RigidBodyComponent>();
	//if (!rigidComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	//glm::vec3 targetPosition = rigidComp->ptr->GetPosition();
	//glm::vec3 targetVelocity = rigidComp->ptr->GetLinearVelocity();

	//glm::vec3 ourPosition = rigidBody->GetPosition();
	//glm::vec3 ourVelocity = rigidBody->GetLinearVelocity();

	//glm::vec3 direction = targetPosition - ourPosition;
	//float distance = glm::length(direction);

	//float speed = glm::length(ourVelocity);
	//float t;
	//if (speed <= distance / maxSteps) // Target is far away
	//{
	//	t = maxSteps;
	//}
	//else // Target is close
	//{
	//	t = distance / speed;
	//}

	//glm::vec3 futurePos = targetPosition + targetVelocity * t;
	//glm::vec3 velocity = glm::normalize(futurePos - ourPosition) * this->speed;

	//glm::vec3 steer = velocity - ourVelocity;
	//steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	//steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	//steer.z = glm::clamp(steer.z, -maxForce, maxForce);

	//return steer;
	return glm::vec3(0.0f);
}