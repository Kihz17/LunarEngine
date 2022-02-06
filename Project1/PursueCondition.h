#pragma once

#include "ISteeringCondition.h"
#include "PursueBehaviour.h"

class PursueCondition : public ISteeringCondition
{
public:
	PursueCondition(PursueBehaviour* behaviour);
	virtual ~PursueCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	Entity* FindTarget(const std::vector<Entity*>& entities);

	PursueBehaviour* behaviour;

};