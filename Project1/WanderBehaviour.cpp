#include "WanderBehaviour.h"
#include "Utils.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <algorithm>

WanderBehaviour::WanderBehaviour(float distanceToCircle, float circleRadius, float speed, float turnSpeed, float maxForce)
	: SteeringBehaviour(SteeringBehaviourType::Normal, speed, turnSpeed, maxForce),
	distanceToCircle(distanceToCircle),
	circleRadius(circleRadius)
{

}

WanderBehaviour::~WanderBehaviour()
{

}

glm::vec3 WanderBehaviour::ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation)
{
	float yaw = glm::yaw(rotation);
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
	steer.x = std::min(steer.x, maxForce);
	steer.y = std::min(steer.y, maxForce);
	steer.z = std::min(steer.z, maxForce);
	return steer;
}