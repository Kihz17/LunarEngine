#pragma once

#include "SteeringBehaviour.h"

class FleeBehaviour : public SteeringBehaviour
{
public:
	FleeBehaviour(Physics::IRigidBody* rigidBody, float fleeRadius = -1.0f, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~FleeBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;

private:
	float fleeRadius;
};