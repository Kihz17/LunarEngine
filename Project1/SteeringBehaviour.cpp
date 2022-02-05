#include "SteeringBehaviour.h"
#include "Components.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

SteeringBehaviour::SteeringBehaviour(Physics::IRigidBody* rigidBody, SteeringBehaviourType type, float speed, float turnSpeed, float maxForce)
	: type(type),
	turnSpeed(turnSpeed),
	maxForce(maxForce),
	speed(speed),
	target(nullptr),
	rigidBody(rigidBody)
{

}

SteeringBehaviour::~SteeringBehaviour()
{

}

void SteeringBehaviour::Update(float deltaTime)
{
	glm::vec3 force = ComputeSteeringForce();
	rigidBody->ApplyForce(force);
	glm::vec3 dir = rigidBody->GetLinearVelocity();
	if (glm::length(dir) != 0.0f)
	{
		LookAtDirection(glm::normalize(dir), deltaTime);
	}
}

void SteeringBehaviour::LookAtDirection(const glm::vec3& direction, float deltaTime)
{
	glm::quat desiredRot = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f)); // Get the quat that is "looking" in the direction
	rigidBody->SetOrientation(glm::slerp(rigidBody->GetOrientation(), desiredRot, deltaTime * turnSpeed)); // Linearly interpolate between the current rotation and desired rotation
}