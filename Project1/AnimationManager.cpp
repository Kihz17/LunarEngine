#include "AnimationManager.h"
#include "Components.h"

#include <glm/gtx/easing.hpp>

#include <iostream>

void AnimationManager::Update(const std::unordered_map<unsigned int, Entity*>& entities, float deltaTime)
{
    AnimationComponent* animComp = nullptr;
    PositionComponent* posComp = nullptr;
    ScaleComponent* scaleComp = nullptr;
    RotationComponent* rotComp = nullptr;

    std::unordered_map<unsigned int, Entity*>::const_iterator it;
    for (it = entities.begin(); it != entities.end(); it++)
    {
        Entity* entity = it->second;

        animComp = entity->GetComponent<AnimationComponent>();
        if (!animComp || !animComp->playing) continue;

        posComp = entity->GetComponent<PositionComponent>();
        if (!animComp)
        {
            std::cout << "Cannot play animation for entity '" << entity->GetName() << "' because it is missing a position component." << std::endl;
            continue;
        }

        scaleComp = entity->GetComponent<ScaleComponent>();
        if (!animComp) 
        {
            std::cout << "Cannot play animation for entity '" << entity->GetName() << "' because it is missing a scale component." << std::endl;
            continue;
        }

        rotComp = entity->GetComponent<RotationComponent>();
        if (!animComp) 
        {
            std::cout << "Cannot play animation for entity '" << entity->GetName() << "' because it is missing a rotation component." << std::endl;
            continue;
        }

        // We have all the components we need, start playing the animation
        animComp->currentTime += deltaTime * animComp->speed;

        if (animComp->currentTime > animComp->duration)
        {
            animComp->currentTime = animComp->duration;
            if (animComp->repeat)
            {
                animComp->currentTime = 0;
            }
            else
            {
                animComp->playing = false;
            }
        }

        // Find active keyframes
        if (animComp->keyFramePositions.empty() || animComp->keyFrameScales.empty() || animComp->keyFrameRotations.empty())
        {
            std::cout << "Cannot play animation for entity '" << entity->GetName() << "' because it contains an empty keyframe vector." << std::endl;
            continue;
        }

        // Update position
        UpdateAnimationPosition(animComp, posComp);

        // Update scale
        UpdateAnimationScale(animComp, scaleComp);

        // Update Rotation
        UpdateAnimationRotation(animComp, rotComp);
    }
}

int AnimationManager::FindKeyFramePositionIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFramePositions.size(); index++)
    {
        if (animComp->keyFramePositions[index].time > time) return index - 1;
    }

    return animComp->keyFramePositions.size() - 1;
}

int AnimationManager::FindKeyFrameScaleIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFrameScales.size(); index++)
    {
        if (animComp->keyFrameScales[index].time > time) return index - 1;
    }

    return animComp->keyFrameScales.size() - 1;
}

int AnimationManager::FindKeyFrameRotationIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFrameRotations.size(); index++)
    {
        if (animComp->keyFrameRotations[index].time > time) return index - 1;
    }

    return animComp->keyFrameRotations.size() - 1;
}

void AnimationManager::UpdateAnimationPosition(AnimationComponent* animComp, PositionComponent* posComp)
{
    // Only 1 keyframe in the animation, just use that
    if (animComp->keyFramePositions.size() == 1)
    {
        posComp->value = animComp->keyFramePositions[0].position;
        return;
    }

    int index = FindKeyFramePositionIndex(animComp, animComp->currentTime);

    // We are at the last keyframe, only use that keyframe
    if (index == animComp->keyFramePositions.size() - 1)
    {
        posComp->value = animComp->keyFramePositions[index].position;
        return;
    }

    // Update postion
    int nextIndex = index + 1;

    // Grab the current and next frame
    const KeyFramePositionComponent& keyFramePos1 = animComp->keyFramePositions[index];
    const KeyFramePositionComponent& keyFramePos2 = animComp->keyFramePositions[nextIndex];

    float positionFraction = (animComp->currentTime - keyFramePos1.time) / (keyFramePos2.time - keyFramePos1.time);

    switch (keyFramePos2.easingType)
    {
    case EaseIn:
        positionFraction = glm::sineEaseIn(positionFraction);
        break;
    case EaseOut:
        positionFraction = glm::sineEaseOut(positionFraction);
        break;
    case EaseInOut:
        positionFraction = glm::sineEaseInOut(positionFraction);
        break;
    default:
        break;
    }

    posComp->value = keyFramePos1.position + (keyFramePos2.position - keyFramePos1.position) * positionFraction;
}

void AnimationManager::UpdateAnimationScale(AnimationComponent* animComp, ScaleComponent* scaleComp)
{
    // Only 1 keyframe in the animation, just use that
    if (animComp->keyFrameScales.size() == 1)
    {
        scaleComp->value = animComp->keyFrameScales[0].scale;
        return;
    }

    int index = FindKeyFrameScaleIndex(animComp, animComp->currentTime);

    // We are at the last keyframe, only use that keyframe
    if (index == animComp->keyFrameScales.size() - 1)
    {
        scaleComp->value = animComp->keyFrameScales[index].scale;
        return;
    }

    // Update postion
    int nextIndex = index + 1;

    // Grab the current and next frame
    const KeyFrameScaleComponent& keyFrameScale1 = animComp->keyFrameScales[index];
    const KeyFrameScaleComponent& keyFrameScale2 = animComp->keyFrameScales[nextIndex];

    float scaleFraction = (animComp->currentTime - keyFrameScale1.time) / (keyFrameScale2.time - keyFrameScale1.time);

    switch (keyFrameScale2.easingType)
    {
    case EaseIn:
        scaleFraction = glm::sineEaseIn(scaleFraction);
        break;
    case EaseOut:
        scaleFraction = glm::sineEaseOut(scaleFraction);
        break;
    case EaseInOut:
        scaleFraction = glm::sineEaseInOut(scaleFraction);
        break;
    default:
        break;
    }

    scaleComp->value = keyFrameScale1.scale + (keyFrameScale2.scale - keyFrameScale2.scale) * scaleFraction;
}

void AnimationManager::UpdateAnimationRotation(AnimationComponent* animComp, RotationComponent* rotComp)
{
    // Only 1 keyframe in the animation, just use that
    if (animComp->keyFrameRotations.size() == 1)
    {
        rotComp->value = animComp->keyFrameRotations[0].rotation;
        return;
    }

    int index = FindKeyFrameRotationIndex(animComp, animComp->currentTime);

    // We are at the last keyframe, only use that keyframe
    if (index == animComp->keyFrameRotations.size() - 1)
    {
        rotComp->value = animComp->keyFrameRotations[index].rotation;
        return;
    }

    // Update postion
    int nextIndex = index + 1;

    // Grab the current and next frame
    const KeyFrameRotationComponent& keyFrameRot1 = animComp->keyFrameRotations[index];
    const KeyFrameRotationComponent& keyFrameRot2 = animComp->keyFrameRotations[nextIndex];

    float rotFraction = (animComp->currentTime - keyFrameRot1.time) / (keyFrameRot2.time - keyFrameRot1.time);

    switch (keyFrameRot2.easingType)
    {
    case EaseIn:
        rotFraction = glm::sineEaseIn(rotFraction);
        break;
    case EaseOut:
        rotFraction = glm::sineEaseOut(rotFraction);
        break;
    case EaseInOut:
        rotFraction = glm::sineEaseInOut(rotFraction);
        break;
    default:
        break;
    }

    if (keyFrameRot2.interpolationType == 1)
    {
        rotComp->value = glm::lerp(keyFrameRot1.rotation, keyFrameRot2.rotation, rotFraction);
    }
    if (keyFrameRot2.interpolationType == 2)
    {
        rotComp->value = glm::slerp(keyFrameRot1.rotation, keyFrameRot2.rotation, rotFraction);
    }
    else
    {
        rotComp->value = keyFrameRot1.rotation + (keyFrameRot2.rotation - keyFrameRot1.rotation) * rotFraction;
    }
}
