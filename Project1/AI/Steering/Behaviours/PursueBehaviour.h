#pragma once

#include "SteeringBehaviour.h"

class PursueBehaviour : public SteeringBehaviour
{
public:
	PursueBehaviour(Physics::IRigidBody* rigidBody, float maxSteps = 1.0f, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~PursueBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.8f, 0.0, 0.0f); }
private:
	float maxSteps;
};