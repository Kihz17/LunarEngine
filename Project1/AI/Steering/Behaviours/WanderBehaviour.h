#pragma once

#include "SteeringBehaviour.h"

class WanderBehaviour : public SteeringBehaviour
{
public:
	WanderBehaviour(Physics::IRigidBody* rigidBody, float distanceToCircle, float circleRadius, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~WanderBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.0f, 0.8, 0.0f); }

private:
	float distanceToCircle;
	float circleRadius;
};