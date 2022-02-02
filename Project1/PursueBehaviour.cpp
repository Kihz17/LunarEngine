#include "PursueBehaviour.h"
#include "Components.h"

PursueBehaviour::PursueBehaviour(float maxSteps, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	maxSteps(maxSteps)
{

}

PursueBehaviour::~PursueBehaviour()
{

}

glm::vec3 PursueBehaviour::ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation)
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
	glm::vec3 velocity = glm::normalize(rigidBody->GetPosition() - futurePos) * speed;

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = std::min(steer.x, maxForce);
	steer.y = std::min(steer.y, maxForce);
	steer.z = std::min(steer.z, maxForce);

	return steer;
}