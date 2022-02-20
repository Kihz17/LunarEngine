#pragma once

#include "SteeringBehaviour.h"
#include "RigidBodyComponent.h"

struct FlockingGroup
{
	std::vector<RigidBodyComponent*> rigidbodies;
};

struct FlockingInfo
{
	FlockingGroup* group;

	float separationRadius;
	float alignmentRadius;
	float cohesionRadius;

	float separationWeight;
	float alignmentWeight;
	float cohesionWeight;
};

class FlockingBehaviour : public SteeringBehaviour
{
public:
	FlockingBehaviour(Physics::IRigidBody* rigidBody, const FlockingInfo& info, float speed = 1.0f, float turnSpeed = 1.0f, float maxForce = 10.0f);
	virtual ~FlockingBehaviour();

	virtual glm::vec3 ComputeSteeringForce() override;
	virtual glm::vec3 GetColor() const override { return glm::vec3(0.8f, 0.0f, 0.0f); }

	float separationRadius;
	float alignmentRadius;
	float cohesionRadius;

	float separationWeight;
	float alignmentWeight;
	float cohesionWeight;

	glm::vec3 moveDirection;
private:
	glm::vec3 Separate();
	glm::vec3 Align();
	glm::vec3 Cohesion();

	FlockingGroup* group;
};