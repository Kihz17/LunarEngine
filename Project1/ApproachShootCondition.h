#pragma once

#include "ISteeringCondition.h"
#include "SeekBehaviour.h"

class ApproachShootCondition : public ISteeringCondition
{
public:
	ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval);
	virtual ~ApproachShootCondition();

	virtual bool CanUse() override;
	virtual bool CanContinueToUse() override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	SeekBehaviour* behaviour;
	float lastShootTime;
	float shootInterval;
};