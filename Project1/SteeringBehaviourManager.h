#pragma once

#include "Entity.h"

#include <unordered_map>

class SteeringBehaviourComponent;
class SteeringBehaviourManager
{
public:
	SteeringBehaviourManager();
	virtual ~SteeringBehaviourManager();

	void Update(const std::unordered_map<unsigned int, Entity*>& entities, float deltaTime);

private:
	void TryActivateBehaviour(SteeringBehaviourComponent* behaviourComp);
};