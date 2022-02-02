#pragma once

#include "SteeringBehaviour.h"

class PursueBehaviour : public SteeringBehaviour
{
public:
	PursueBehaviour(float maxSteps = 1.0f, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~PursueBehaviour();

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) override;

private:
	float maxSteps;
};