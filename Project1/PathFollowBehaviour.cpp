#include "PathFollowBehaviour.h"

PathFollowBehaviour::PathFollowBehaviour(Physics::IRigidBody* rigidBody, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce)
{

}

PathFollowBehaviour::~PathFollowBehaviour()
{

}

glm::vec3 PathFollowBehaviour::ComputeSteeringForce()
{
	glm::vec3& targetPos = nodes[currentNode];
	if (nodes.size() > 1)
	{
		float distance = glm::length(rigidBody->GetPosition() - targetPos);
		if (distance < pathRadius) // We have arrived, move to next
		{
			currentNode += pathDirection;
			if (repeating) // Check if we should start back at the beginning
			{
				if (pathDirection > 0 && currentNode >= nodes.size())
				{
					currentNode = currentNode - nodes.size();
				}
				else if (pathDirection < 0 && currentNode < 0)
				{
					currentNode = nodes.size() + currentNode;
				}
			}
		}
	}

	glm::vec3 direction = targetPos - rigidBody->GetPosition();
	glm::vec3 velocity = glm::normalize(direction) * speed;
	float distance = glm::length(direction);
	if (distance < arriveRadius) // Slow down on arrival
	{
		velocity *= ((distance - 0.75f) / arriveRadius);
	}

	if (glm::length(velocity) > maxForce)
	{
		velocity = glm::normalize(velocity) * maxForce;
	}

	return velocity;
}