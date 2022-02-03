#include "SteeringBehaviour.h"
#include "Components.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

SteeringBehaviour::SteeringBehaviour(SteeringBehaviourType type, float speed, float turnSpeed, float maxForce)
	: type(type),
	turnSpeed(turnSpeed),
	maxForce(maxForce),
	speed(speed),
	target(nullptr)
{

}

SteeringBehaviour::~SteeringBehaviour()
{

}

void SteeringBehaviour::Update(Physics::IRigidBody* rigidBody, glm::quat& rotation, float deltaTime)
{
	glm::vec3 force = ComputeSteeringForce(rigidBody, rotation);
	rigidBody->ApplyForce(force);
	LookAtDirection(rigidBody->GetLinearVelocity(), rotation, deltaTime);
}

void SteeringBehaviour::LookAtDirection(const glm::vec3& direction, glm::quat& rotation, float deltaTime)
{
	glm::quat desiredRot = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f)); // Get the quat that is "looking" in the direction
	rotation = glm::slerp(rotation, desiredRot, deltaTime * turnSpeed); // Linearly interpolate between the current rotation and desired rotation
}