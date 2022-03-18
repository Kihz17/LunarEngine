#pragma once 

#include "ISteeringBehaviour.h"

#include <functional>

class RotationComponent;
class SteeringBehaviour : public ISteeringBehaviour
{
public:
	SteeringBehaviour(Physics::IRigidBody* rigidBody,  SteeringBehaviourType type, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~SteeringBehaviour();

	virtual void Update(float deltaTime) override;
	virtual Entity* GetTarget() override { return target; }
	virtual void SetTarget(Entity* entity) override { target = entity; }
	virtual SteeringBehaviourType GetType() const override { return type; }
	virtual Physics::IRigidBody* GetRigidBody() override { return rigidBody; }

	virtual glm::vec3 ComputeSteeringForce() = 0;

	virtual void LookAtDirection(const glm::vec3& direction, float deltaTime);

protected:
	float turnSpeed;
	float maxForce;
	float speed;

	Entity* target;
	SteeringBehaviourType type;

	Physics::IRigidBody* rigidBody;
};