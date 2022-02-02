#pragma once

#include "ApplicationLayer.h"
#include "Entity.h"
#include "IKeyFrameListener.h"

#include <glm/glm.hpp>
#include <unordered_map>

class AILayer : public ApplicationLayer
{
public:
	AILayer(const std::unordered_map<unsigned int, Entity*>& entities);
	virtual ~AILayer();

	virtual void OnUpdate(float deltaTime) override;

private:
	void TryActivateBehaviour(SteeringBehaviourComponent* behaviourComp);

	const std::unordered_map<unsigned int, Entity*>& entities;
};