#pragma once

#include "SteeringBehaviour.h"

class PursueBehaviour : public SteeringBehaviour
{
public:
	PursueBehaviour(Physics::IRigidBody* rigidBody, float maxSteps = 1.0f, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~PursueBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;

private:
	float maxSteps;
};