#include "AILayer.h"
#include "Components.h"

#include <algorithm>
#include <iostream>

AILayer::AILayer(const std::vector<Entity*>& entities)
    : entities(entities)
{

}

AILayer::~AILayer()
{

}

void AILayer::OnUpdate(float deltaTime)
{
    RigidBodyComponent* rigidComp = nullptr;
    RotationComponent* rotComp = nullptr;
    SteeringBehaviourComponent* behaviourComp = nullptr;
    for (Entity* entity : entities)
    {
        if (!entity->IsValid()) continue;
        behaviourComp = entity->GetComponent<SteeringBehaviourComponent>();
        if (!behaviourComp) continue;

        rigidComp = entity->GetComponent<RigidBodyComponent>();
        if (!rigidComp)
        {
            std::cout << "Cannot update steering behaviour for entity '" << entity->GetName() << "' because it is missing a rigidbody component!" << std::endl;
            continue;
        }

        rotComp = entity->GetComponent<RotationComponent>();
        if (!rotComp)
        {
            std::cout << "Cannot update steering behaviour for entity '" << entity->GetName() << "' because it is missing a rotation component!" << std::endl;
            continue;
        }

        if (behaviourComp->active && !behaviourComp->active->CanContinueToUse(entities)) // This behaviour can no longer be used, stop it
        {
            behaviourComp->active->OnStop();
            behaviourComp->active = nullptr;
            behaviourComp->activePriority = -1;
        }

        if (!behaviourComp->active) // No behaviour active, try to start one
        {
            TryActivateBehaviour(behaviourComp);
        }
        else // We have an active behaviour
        {
            bool shouldStop = !behaviourComp->active->CanContinueToUse(entities);

            if (shouldStop) // The active behaviour can no longer be used, search for a new one to activate
            {
                behaviourComp->active->OnStop();
                behaviourComp->active = nullptr;
                behaviourComp->activePriority = -1;
                TryActivateBehaviour(behaviourComp);
            }
            else // Active behaviour is still running, check if any behaviours with a higher priority can run
            {
                std::vector<ISteeringCondition*>& targeting = behaviourComp->targetingBehaviours;
                int checkIndex = behaviourComp->active->GetBehaviour()->GetType() == SteeringBehaviourType::Normal ? (int)targeting.size() : std::min(behaviourComp->activePriority, (int) targeting.size());
                for (int i = 0; i < checkIndex; i++)
                {
                    if (targeting[i] && targeting[i]->CanUse(entities)) // We can use this!
                    {
                        targeting[i]->OnStart();
                        behaviourComp->active = targeting[i];
                        behaviourComp->activePriority = i;
                        break;
                    }
                }

                SteeringBehaviourType type = behaviourComp->active->GetBehaviour()->GetType();
                if (type != SteeringBehaviourType::Targeting) // Only check "normal" behaviours if the active one is not targeting
                {
                    std::vector<ISteeringCondition*>& normal = behaviourComp->behaviours;
                    checkIndex = std::min(behaviourComp->activePriority, (int)normal.size());
                    for (int i = 0; i < checkIndex; i++)
                    {
                        if (normal[i] && normal[i]->CanUse(entities)) // We can use this!
                        {
                            normal[i]->OnStart();
                            behaviourComp->active = normal[i];
                            behaviourComp->activePriority = i;
                            break;
                        }
                    }
                }
            }
        }

        // Update the active behaviour
        if (behaviourComp->active)
        {
            behaviourComp->active->GetBehaviour()->Update(deltaTime); // Update the actual behaviour
            behaviourComp->active->Update(deltaTime); // Update the behaviour condition
        }
    }
}

void AILayer::TryActivateBehaviour(SteeringBehaviourComponent* behaviourComp)
{
    //Check targeting first
    std::vector<ISteeringCondition*>& targeting = behaviourComp->targetingBehaviours;
    for (int i = 0; i < targeting.size(); i++)
    {
        if (targeting[i] && targeting[i]->CanUse(entities)) // We can use this!
        {
            targeting[i]->OnStart();
            behaviourComp->active = targeting[i];
            behaviourComp->activePriority = i;
            return;
        }
    }

    // We havent found a behaviour to run yet, check the non-targeting behaviours
    std::vector<ISteeringCondition*>& normal = behaviourComp->behaviours;
    for (int i = 0; i < normal.size(); i++)
    {
        if (normal[i] && normal[i]->CanUse(entities)) // We can use this!
        {
            normal[i]->OnStart();
            behaviourComp->active = normal[i];
            behaviourComp->activePriority = i;
            return;
        }
    }
}