#pragma once

#include "SteeringBehaviour.h"

class FleeBehaviour : public SteeringBehaviour
{
public:
	FleeBehaviour(float fleeRadius = -1.0f, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~FleeBehaviour();

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) override;

private:
	float fleeRadius;
};