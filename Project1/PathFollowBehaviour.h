#pragma once

#include "SteeringBehaviour.h"

class PathFollowBehaviour : public SteeringBehaviour
{
public:
	PathFollowBehaviour(Physics::IRigidBody* rigidBody, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~PathFollowBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.0f, 0.0, 0.8f); }

private:
	int currentNode;
	std::vector<glm::vec3> nodes;
	bool repeating;
	float arriveRadius;
	float pathRadius;
	int pathDirection;
};