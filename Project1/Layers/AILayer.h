#pragma once

#include "ApplicationLayer.h"
#include "Entity.h"
#include "IKeyFrameListener.h"

#include <glm/glm.hpp>
#include <unordered_map>

class AILayer : public ApplicationLayer
{
public:
	AILayer(const std::vector<Entity*>& entities);
	virtual ~AILayer();

	virtual void OnUpdate(float deltaTime) override;

	template <class T, typename... Args> T* CreateBehaviour(Args&&... args)
	{
		T* newBehaviour = new T(std::forward<Args>(args)...);
		assert(newBehaviour);

		if (!dynamic_cast<ISteeringBehaviour*>(newBehaviour)) return nullptr;

		activeBehaviours.push_back(newBehaviour);
		return newBehaviour;
	}

private:
	friend class SteeringEntityRemoveListener;

	void TryActivateBehaviour(SteeringBehaviourComponent* behaviourComp);

	const std::vector<Entity*>& entities;
	std::vector<ISteeringBehaviour*> activeBehaviours;
};