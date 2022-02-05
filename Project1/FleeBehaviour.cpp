#include "FleeBehaviour.h"
#include "Components.h"

FleeBehaviour::FleeBehaviour(Physics::IRigidBody* rigidBody, float speed, float turnSpeed, float maxForce, float fleeRadius)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	fleeRadius(fleeRadius)
{

}

FleeBehaviour::~FleeBehaviour()
{

}

glm::vec3 FleeBehaviour::ComputeSteeringForce()
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	PositionComponent* posComp = target->GetComponent<PositionComponent>();
	if (!posComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = rigidBody->GetPosition() - posComp->value;

	bool shouldFlee = fleeRadius == -1.0f || glm::length(direction) <= fleeRadius;
	if (!shouldFlee) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 velocity = glm::normalize(direction) * speed;

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);
	return steer;
}