#pragma once

#include "ISteeringCondition.h"
#include "IdleBehaviour.h"

class IdleCondition : public ISteeringCondition
{
public:
	IdleCondition(IdleBehaviour* behaviour, float useTime, float waitTime);
	virtual ~IdleCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	IdleBehaviour* behaviour;
	float useTime;
	float waitTime;

	float startTime;
	float endTime;
};