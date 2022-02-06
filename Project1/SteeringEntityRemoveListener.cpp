#include "SteeringEntityRemoveListener.h"
#include "SteeringBehaviourComponent.h"

SteeringEntityRemoveListener::SteeringEntityRemoveListener(AILayer* layer)
	:activeBehaviours(layer->activeBehaviours)
{

}

void SteeringEntityRemoveListener::OnEntityRemove(Entity* entity)
{
	//SteeringBehaviourComponent* steeringComp = entity->GetComponent<SteeringBehaviourComponent>();
	//if (steeringComp) // This entity has a steering behaviour, make sure to delete the behaviours that belong to it
	//{
	//	for (ISteeringCondition* condition : steeringComp->targetingBehaviours) RemoveBehaviour(condition->GetBehaviour());
	//	for (ISteeringCondition* condition : steeringComp->behaviours) RemoveBehaviour(condition->GetBehaviour());
	//}

	// Check if any active behaviours has the entity to remove as a target
	for (ISteeringBehaviour* activeBehaviour : activeBehaviours)
	{
		if (activeBehaviour->GetTarget() == entity) activeBehaviour->SetTarget(nullptr);
	}
}

void SteeringEntityRemoveListener::RemoveBehaviour(ISteeringBehaviour* behaviour)
{
	int removeIndex = -1;
	for (int i = 0; i < activeBehaviours.size(); i++)
	{
		if (activeBehaviours[i] == behaviour)
		{
			removeIndex = i;
			break;
		}
	}

	activeBehaviours.erase(activeBehaviours.begin() + removeIndex);
	delete behaviour;
}