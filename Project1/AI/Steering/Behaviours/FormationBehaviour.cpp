#include "FormationBehaviour.h"

FormationBehaviour::FormationBehaviour(Physics::IRigidBody* rigidBody, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Normal, speed, turnSpeed, maxForce)
{

}

FormationBehaviour::~FormationBehaviour()
{

}

glm::vec3 FormationBehaviour::ComputeSteeringForce()
{
	glm::vec3 velocity = glm::normalize(targetPos - rigidBody->GetPosition()) * speed;
	if (glm::length(velocity) > maxForce)
	{
		velocity = glm::normalize(velocity) * maxForce;
	}
	return velocity;
}