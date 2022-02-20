#pragma once

#include "ISteeringCondition.h"
#include "FlockingBehaviour.h"

class FlockingCondition : public ISteeringCondition
{
public:
	FlockingCondition(FlockingBehaviour* behaviour);
	virtual ~FlockingCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

	bool canUse;

private:
	FlockingBehaviour* behaviour;

};