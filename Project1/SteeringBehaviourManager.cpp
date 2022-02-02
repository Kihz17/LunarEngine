#include "SteeringBehaviourManager.h"
#include "Components.h"

#include <iostream>

SteeringBehaviourManager::SteeringBehaviourManager()
{

}

SteeringBehaviourManager::~SteeringBehaviourManager()
{

}

void SteeringBehaviourManager::Update(const std::unordered_map<unsigned int, Entity*>& entities, float deltaTime)
{
    RigidBodyComponent* rigidComp = nullptr;
    RotationComponent* rotComp = nullptr;
    SteeringBehaviourComponent* behaviourComp = nullptr;
    std::unordered_map<unsigned int, Entity*>::const_iterator it;
    //for (it = entities.begin(); it != entities.end(); it++)
    //{
    //    Entity* entity = it->second;
    //    behaviourComp = entity->GetComponent<SteeringBehaviourComponent>();
    //    if (!behaviourComp) continue;

    //    rigidComp = entity->GetComponent<RigidBodyComponent>();
    //    if (!rigidComp)
    //    {
    //        std::cout << "Cannot update steering behaviour for entity '" << entity->GetName() << "' because it is missing a rigidbody component!" << std::endl;
    //        continue;
    //    }

    //    rotComp = entity->GetComponent<RotationComponent>();
    //    if (!rotComp)
    //    {
    //        std::cout << "Cannot update steering behaviour for entity '" << entity->GetName() << "' because it is missing a rotation component!" << std::endl;
    //        continue;
    //    }

    //    ISteeringBehaviour* activeBehaviour = behaviourComp->active;
    //    int activePrio = behaviourComp->activePriority;

    //    if (!activeBehaviour->CanContinueToUse()) // This behaviour can no longer be used, stop it
    //    {
    //        behaviourComp->active = nullptr;
    //        behaviourComp->activePriority = -1;
    //    }

    //    if (behaviourComp->active == nullptr) // No behaviour active, try to start one
    //    {
    //        TryActivateBehaviour(behaviourComp);
    //    }
    //    else // We have an active behaviour
    //    {
    //        bool shouldStop = !activeBehaviour->CanContinueToUse();
    //        int targetSearchIndex = shouldStop ? behaviourComp->targetingBehaviours.size() : activePrio;
    //        int normalSearchIndex = shouldStop ? behaviourComp->behaviours.size() : activePrio;

    //        if (shouldStop) // The active behaviour can no longer be used, search for a new one to activate
    //        {
    //            behaviourComp->active->OnStop();
    //            behaviourComp->active = nullptr;
    //            behaviourComp->activePriority = -1;
    //            TryActivateBehaviour(behaviourComp);
    //        }
    //        else // Active behaviour is still running, check if any behaviours with a higher priority can run
    //        {
    //            std::vector<ISteeringBehaviour*>& targeting = behaviourComp->targetingBehaviours;
    //            for (int i = 0; i < activePrio; i++)
    //            {
    //                if (targeting[i]->CanUse()) // We can use this!
    //                {
    //                    targeting[i]->OnStart();
    //                    behaviourComp->active = targeting[i];
    //                    behaviourComp->activePriority = i;
    //                    break;
    //                }
    //            }

    //            SteeringBehaviourType type = behaviourComp->active->GetType();
    //            if (type != SteeringBehaviourType::Targeting) // Only check "normal" behaviours if the active one is not targeting
    //            {
    //                std::vector<ISteeringBehaviour*>& normal = behaviourComp->targetingBehaviours;
    //                for (int i = 0; i < activePrio; i++)
    //                {
    //                    if (normal[i]->CanUse()) // We can use this!
    //                    {
    //                        normal[i]->OnStart();
    //                        behaviourComp->active = normal[i];
    //                        behaviourComp->activePriority = i;
    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //    }

    //    // Update the active behaviour
    //    if (behaviourComp->active)
    //    {
    //        behaviourComp->active->Update(rigidComp->ptr, rotComp->value, deltaTime);
    //    }
    //}
}

void SteeringBehaviourManager::TryActivateBehaviour(SteeringBehaviourComponent* behaviourComp)
{
    // Check targeting first
    //std::vector<ISteeringBehaviour*>& targeting = behaviourComp->targetingBehaviours;
    //for (int i = 0; i < targeting.size(); i++)
    //{
    //    if (targeting[i]->CanUse()) // We can use this!
    //    {
    //        targeting[i]->OnStart();
    //        behaviourComp->active = targeting[i];
    //        behaviourComp->activePriority = i;
    //        return;
    //    }
    //}

    //// We havent found a behaviour to run yet, check the non-targeting behaviours
    //std::vector<ISteeringBehaviour*>& normal = behaviourComp->behaviours;
    //for (int i = 0; i < normal.size(); i++)
    //{
    //    if (normal[i]->CanUse()) // We can use this!
    //    {
    //        normal[i]->OnStart();
    //        behaviourComp->active = normal[i];
    //        behaviourComp->activePriority = i;
    //        return;
    //    }
    //}
}