#pragma once

#include "ISteeringCondition.h"
#include "FleeBehaviour.h"

class FleeCondition : public ISteeringCondition
{
public:
	FleeCondition(FleeBehaviour* behaviour);
	virtual ~FleeCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	Entity* FindTarget(const std::vector<Entity*>& entities);

	FleeBehaviour* behaviour;
};