#pragma once

#include "ISteeringBehaviour.h"

class ISteeringCondition
{
public:
	virtual ~ISteeringCondition() = default;

	virtual bool CanUse() = 0;
	virtual bool CanContinueToUse() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void OnStart() = 0;
	virtual void OnStop() = 0;
	virtual ISteeringBehaviour* GetBehaviour() = 0;
};