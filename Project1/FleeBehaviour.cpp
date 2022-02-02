#include "FleeBehaviour.h"
#include "Components.h"

FleeBehaviour::FleeBehaviour(float fleeRadius, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	fleeRadius(fleeRadius)
{

}

FleeBehaviour::~FleeBehaviour()
{

}

glm::vec3 FleeBehaviour::ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation)
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	PositionComponent* posComp = target->GetComponent<PositionComponent>();
	if (!posComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = rigidBody->GetPosition() - posComp->value;

	bool shouldFlee = fleeRadius == -1.0f || glm::length(direction) <= fleeRadius;
	if (!shouldFlee) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 velocity = glm::normalize(direction) * speed;

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = std::min(steer.x, maxForce);
	steer.y = std::min(steer.y, maxForce);
	steer.z = std::min(steer.z, maxForce);
	return steer;
}