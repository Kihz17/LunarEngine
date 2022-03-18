#pragma once

#include "SteeringBehaviour.h"

class FleeBehaviour : public SteeringBehaviour
{
public:
	FleeBehaviour(Physics::IRigidBody* rigidBody, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f, float fleeRadius = -1.0f);
	virtual ~FleeBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.8f, 0.8f, 0.0f); }

private:
	float fleeRadius;
};