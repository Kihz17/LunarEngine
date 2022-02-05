#pragma once

#include "IEntityRemoveListener.h"
#include "ISteeringBehaviour.h"
#include "AILayer.h"

#include <vector>

class SteeringEntityRemoveListener : public IEntityRemoveListener
{
public:
	SteeringEntityRemoveListener(AILayer* layer);

	virtual void OnEntityRemove(Entity* entity) override;

private:
	void RemoveBehaviour(ISteeringBehaviour* behaviour);

	std::vector<ISteeringBehaviour*>& activeBehaviours;
};