#pragma once

#include "ISteeringCondition.h"
#include "SeekBehaviour.h"

class ApproachShootCondition : public ISteeringCondition
{
public:
	ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval);
	virtual ~ApproachShootCondition();

	virtual bool CanUse(const std::unordered_map<unsigned int, Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::unordered_map<unsigned int, Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	Entity* FindTarget(const std::unordered_map<unsigned int, Entity*>& entities);

	SeekBehaviour* behaviour;
	float lastShootTime;
	float shootInterval;
};