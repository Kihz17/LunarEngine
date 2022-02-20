#pragma once

#include "ISteeringCondition.h"
#include "FormationBehaviour.h"

class FormationCondition : public ISteeringCondition
{
public:
	FormationCondition(FormationBehaviour* behaviour);
	virtual ~FormationCondition();

	virtual bool CanUse(const std::vector<Entity*>& entities) override;
	virtual bool CanContinueToUse(const std::vector<Entity*>& entities) override;
	virtual void Update(float deltaTime) override;
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual ISteeringBehaviour* GetBehaviour() override { return behaviour; }

private:
	FormationBehaviour* behaviour;

};