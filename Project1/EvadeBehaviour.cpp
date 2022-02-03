#include "EvadeBehaviour.h"
#include "Components.h"

EvadeBehaviour::EvadeBehaviour(Physics::IRigidBody* rigidBody,  float maxSteps, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	maxSteps(maxSteps)
{

}

EvadeBehaviour::~EvadeBehaviour()
{

}

glm::vec3 EvadeBehaviour::ComputeSteeringForce()
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	PositionComponent* posComp = target->GetComponent<PositionComponent>();
	if (!posComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	RigidBodyComponent* rigidComp = target->GetComponent<RigidBodyComponent>();
	if (!rigidComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = posComp->value - rigidBody->GetPosition();
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

	glm::vec3 futurePos = posComp->value + rigidComp->ptr->GetLinearVelocity() * t;
	glm::vec3 velocity = glm::normalize(futurePos - rigidBody->GetPosition()) * speed;

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);

	return steer;
}