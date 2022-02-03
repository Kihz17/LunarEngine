#pragma once

#include "SteeringBehaviour.h"

enum class SeekType
{
	None, 
	Approach,
	Stop
};

class SeekBehaviour : public SteeringBehaviour
{
public:
	SeekBehaviour(float arriveRadius, SeekType type, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~SeekBehaviour();

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) override;

	bool IsWithinRadius() const { return withinRadius; }

private:
	float arriveRadius;
	bool withinRadius;
	SeekType type;
};