#pragma once

#include "ISteeringCondition.h"
#include "WanderBehaviour.h"

class WanderCondition : public ISteeringCondition
{
public:
	WanderCondition(WanderBehaviour* behaviour, float useTime, float waitTime);
	virtual ~WanderCondition();

	virtual bool CanUse(const std::unordered_map<unsigned int, Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	WanderBehaviour* behaviour;
	float useTime;
	float waitTime;

	float startTime;
	float endTime;
};