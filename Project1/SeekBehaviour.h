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
	SeekBehaviour(Physics::IRigidBody* rigidBody, float arriveRadius, SeekType type, bool maintainRadius = false, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~SeekBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;

	bool IsWithinRadius() const { return withinRadius; }

private:
	float arriveRadius;
	bool withinRadius;
	SeekType type;
	bool maintainRadius;
};