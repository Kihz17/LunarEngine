#pragma once

#include "SteeringBehaviour.h"

class WanderBehaviour : public SteeringBehaviour
{
public:
	WanderBehaviour(float distanceToCircle, float circleRadius, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~WanderBehaviour();

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) override;

private:
	float distanceToCircle;
	float circleRadius;
};