#pragma once 

#include "ISteeringBehaviour.h"

#include <functional>

class RotationComponent;
class SteeringBehaviour : public ISteeringBehaviour
{
public:
	SteeringBehaviour(SteeringBehaviourType type, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~SteeringBehaviour();

	virtual void Update(Physics::IRigidBody* rigidBody, glm::quat& rotation, float deltaTime) override;
	virtual Entity* GetTarget() override { return target; }
	virtual void SetTarget(Entity* entity) override { target = entity; }
	virtual SteeringBehaviourType GetType() const override { return type; }

	virtual glm::vec3 ComputeSteeringForce(Physics::IRigidBody* rigidBody, glm::quat& rotation) = 0;

	virtual void LookAtDirection(const glm::vec3& direction, glm::quat& rotation, float deltaTime);

protected:
	float turnSpeed;
	float maxForce;
	float speed;

	Entity* target;
	SteeringBehaviourType type;
};