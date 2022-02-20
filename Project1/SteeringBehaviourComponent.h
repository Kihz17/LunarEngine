#pragma once

#include "Component.h"
#include "ISteeringCondition.h"

#include <vector>

struct SteeringBehaviourComponent : public Component
{
	SteeringBehaviourComponent(std::vector<ISteeringCondition*> behaviours) : behaviours(behaviours) {}
	SteeringBehaviourComponent() {}

	virtual ~SteeringBehaviourComponent()
	{
		for (ISteeringCondition* condition : targetingBehaviours) delete condition;
		for (ISteeringCondition* condition : behaviours) delete condition;
	}

	virtual void ImGuiUpdate() override
	{

	}

	void AddBehaviour(int priority, ISteeringCondition* behaviour)
	{
		if (behaviour->GetBehaviour()->GetType() != SteeringBehaviourType::Normal) return;

		int prioSize = priority + 1;
		if (prioSize > behaviours.size())
		{
			behaviours.resize(prioSize);
		}

		behaviours[priority] = behaviour;
	}

	void AddTargetingBehaviour(int priority, ISteeringCondition* behaviour)
	{
		if (behaviour->GetBehaviour()->GetType() != SteeringBehaviourType::Targeting) return;

		int prioSize = priority + 1;
		if (prioSize > targetingBehaviours.size())
		{
			targetingBehaviours.resize(prioSize);
		}

		targetingBehaviours[priority] = behaviour;
	}

	ISteeringCondition* GetBehaviour(int priority, SteeringBehaviourType type)
	{
		if (type == SteeringBehaviourType::Targeting)
		{
			if (priority >= targetingBehaviours.size()) return nullptr;
			return targetingBehaviours[priority];
		}
		else if (type == SteeringBehaviourType::Normal)
		{
			if (priority >= behaviours.size()) return nullptr;
			return behaviours[priority];
		}

		return nullptr;
	}

	ISteeringCondition* active = nullptr;
	int activePriority = -1;

	std::vector<ISteeringCondition*> behaviours;
	std::vector<ISteeringCondition*> targetingBehaviours;
};