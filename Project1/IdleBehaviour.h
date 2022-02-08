#pragma once

#include "SteeringBehaviour.h"

class IdleBehaviour : public SteeringBehaviour
{
public:
	IdleBehaviour(Physics::IRigidBody* rigidBody);
	virtual ~IdleBehaviour();

	virtual void Update(float deltaTime) override;
	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.8f, 0.8f, 0.8f); }
};