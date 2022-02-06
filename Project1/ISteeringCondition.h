#pragma once

#include "ISteeringBehaviour.h"

#include "Entity.h"
#include <vector>

class ISteeringCondition
{
public:
	virtual ~ISteeringCondition() = default;

	virtual bool CanUse(const std::vector<Entity*>& entities) = 0;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void OnStart() = 0;
	virtual void OnStop() = 0;
	virtual ISteeringBehaviour* GetBehaviour() = 0;
};