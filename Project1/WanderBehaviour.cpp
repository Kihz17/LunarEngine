#include "WanderBehaviour.h"
#include "Utils.h"
#include "Components.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <algorithm>

WanderBehaviour::WanderBehaviour(Physics::IRigidBody* rigidBody, float distanceToCircle, float circleRadius, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Normal, speed, turnSpeed, maxForce),
	distanceToCircle(distanceToCircle),
	circleRadius(circleRadius)
{

}

WanderBehaviour::~WanderBehaviour()
{

}

glm::vec3 WanderBehaviour::ComputeSteeringForce()
{
	float yaw = glm::yaw(rigidBody->GetOrientation());
	glm::vec3 pos = rigidBody->GetPosition();
	glm::vec3 center = pos + glm::vec3(cos(-yaw), 0.0f, sin(-yaw)) * distanceToCircle;

	// Get a random point on the edge of the circle
	float randAngle = Utils::RandFloat(0.0f, Utils::PI() * 2.0f);
	//std::cout << "Rand angle: " << randAngle << "\n";
	float x = sin(randAngle) * circleRadius;
	float z = cos(randAngle) * circleRadius;

	glm::vec3 targetPos = glm::vec3(center.x + x, pos.y, center.z + z);
	//std::cout << "Target: " << targetPos.x << " " << targetPos.y << " " << targetPos.z << "\n";
	glm::vec3 velocity = glm::normalize(targetPos - pos) * speed;
	glm::vec3 steer = velocity - rigidBody->GetLinearVelocity();
	steer.x = glm::clamp(steer.x, -maxForce, maxForce);
	steer.y = glm::clamp(steer.y, -maxForce, maxForce);
	steer.z = glm::clamp(steer.z, -maxForce, maxForce);
	return steer;
}