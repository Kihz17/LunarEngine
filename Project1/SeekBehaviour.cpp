#include "SeekBehaviour.h"
#include "Components.h"

SeekBehaviour::SeekBehaviour(Physics::IRigidBody* rigidBody, float slowingRadius, SeekType type, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	arriveRadius(slowingRadius),
	type(type),
	withinRadius(false)
{

}

SeekBehaviour::~SeekBehaviour()
{

}

glm::vec3 SeekBehaviour::ComputeSteeringForce()
{
	if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	PositionComponent* posComp = target->GetComponent<PositionComponent>();
	if(!posComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 direction = posComp->value - rigidBody->GetPosition();
	glm::vec3 velocity = glm::normalize(direction) * speed;
	
	float distance = glm::length(direction);
	withinRadius = distance < arriveRadius;
	if (type == SeekType::Approach && withinRadius) // Slowly approach when in radius
	{
		velocity *= ((distance - 0.75f) / arriveRadius);
	}
	else if (type == SeekType::Stop) // Stop when in radius
	{
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);
	return steer;
}