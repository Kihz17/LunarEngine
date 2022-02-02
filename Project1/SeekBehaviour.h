#pragma once

#include "SteeringBehaviour.h"

class SeekBehaviour : public SteeringBehaviour
{
public:
	SeekBehaviour(float arriveRadius, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~SeekBehaviour();

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) override;

private:
	float arriveRadius;
};