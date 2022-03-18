#pragma once

#include "Entity.h"

#include <IRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

enum class SteeringBehaviourType
{
	Normal,
	Targeting
};

class ISteeringBehaviour
{
public:
	virtual ~ISteeringBehaviour() = default;

	virtual void Update(float deltaTime) = 0;

	virtual Entity* GetTarget() = 0;
	virtual void SetTarget(Entity* entity) = 0;

	virtual Physics::IRigidBody* GetRigidBody() = 0;

	virtual SteeringBehaviourType GetType() const = 0;
	virtual glm::vec3 GetColor() const = 0;
};