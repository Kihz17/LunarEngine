#pragma once

#include "SteeringBehaviour.h"

class FormationBehaviour : public SteeringBehaviour
{
public:
	FormationBehaviour(Physics::IRigidBody* rigidBody, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~FormationBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.8f, 0.8f, 0.0f); }

	glm::vec3 targetPos;
};