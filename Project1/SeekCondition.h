#pragma once

#include "ISteeringCondition.h"
#include "SeekBehaviour.h"

class SeekCondition : public ISteeringCondition
{
public:
	SeekCondition(SeekBehaviour* behaviour);
	virtual ~SeekCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	Entity* FindTarget(const std::vector<Entity*>& entities);

	SeekBehaviour* behaviour;
};