#include "SeekBehaviour.h"
#include "Components.h"

SeekBehaviour::SeekBehaviour(float arriveRadius, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	arriveRadius(arriveRadius)
{

}

SeekBehaviour::~SeekBehaviour()
{

}

glm::vec3 SeekBehaviour::ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation)
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	PositionComponent* posComp = target->GetComponent<PositionComponent>();
	if(!posComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = posComp->value - rigidBody->GetPosition();
	glm::vec3 velocity = glm::normalize(direction) * speed;
	
	float distance = glm::length(direction);
	if (distance < arriveRadius)
	{
		velocity *= ((distance - 0.75f) / arriveRadius);
	}

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = std::min(steer.x, maxForce);
	steer.y = std::min(steer.y, maxForce);
	steer.z = std::min(steer.z, maxForce);
	return steer;
}