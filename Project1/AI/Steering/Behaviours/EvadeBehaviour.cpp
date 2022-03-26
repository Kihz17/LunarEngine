#include "EvadeBehaviour.h"
#include "Components.h"
#include "Utils.h"

#include <glm/gtx/rotate_vector.hpp>

EvadeBehaviour::EvadeBehaviour(Physics::IRigidBody* rigidBody, float angleRandomizer, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Targeting, speed, turnSpeed, maxForce),
	angleRandomizer(angleRandomizer),
	flipDir(false)
{

}

EvadeBehaviour::~EvadeBehaviour()
{

}

glm::vec3 EvadeBehaviour::ComputeSteeringForce()
{
	/*if (!target) return glm::vec3(0.0f, 0.0f, 0.0f);

	RigidBodyComponent* rigidComp = target->GetComponent<RigidBodyComponent>();
	if (!rigidComp) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 targetPosition = rigidComp->ptr->GetPosition();
	glm::vec3 targetVelocity = rigidComp->ptr->GetLinearVelocity();

	glm::vec3 ourPosition = rigidBody->GetPosition();
	glm::vec3 ourVelocity = rigidBody->GetLinearVelocity();

	glm::vec3 moveDirection = glm::cross(glm::normalize(targetVelocity), glm::vec3(0.0f, 1.0f, 0.0f));
	moveDirection = glm::rotateY(moveDirection, Utils::RandFloat(-angleRandomizer, angleRandomizer));
	if (flipDir) moveDirection *= -1.0f;

	glm::vec3 velocity = moveDirection * speed;

	glm::vec3 steer = velocity - ourVelocity;
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);

	return steer;*/
	return glm::vec3(0.0f);
}