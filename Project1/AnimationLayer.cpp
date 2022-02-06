#include "AnimationLayer.h"
#include "Components.h"
#include "KeyFrameListener.h"

#include <glm/gtx/easing.hpp>

#include <iostream>

const glm::vec3 red = glm::vec3(0.8f, 0.0f, 0.0f);
const glm::vec3 yellow = glm::vec3(0.4f, 0.4f, 0.0f);
const glm::vec3 green = glm::vec3(0.0f, 0.8f, 0.0f);
const glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);

AnimationLayer::AnimationLayer(const std::vector<Entity*>& entities)
    : paused(false),
    keyFrameListener(new KeyFrameListener()),
    entities(entities)
{

}

AnimationLayer::~AnimationLayer()
{
    delete keyFrameListener;
}

void AnimationLayer::OnUpdate(float deltaTime)
{
    if (paused) return;

    AnimationComponent* animComp = nullptr;
    PositionComponent* posComp = nullptr;
    ScaleComponent* scaleComp = nullptr;
    RotationComponent* rotComp = nullptr;

    for (Entity* entity : entities)
    {
        if (!entity->IsValid()) continue;
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
        glm::vec3 posColor = glm::vec3(1.0f);
        UpdateAnimationPosition(animComp, posComp, posColor);

        // Update scale
        glm::vec3 scaleColor = glm::vec3(1.0f);
        UpdateAnimationScale(animComp, scaleComp, scaleColor);

        // Update Rotation
        glm::vec3 rotColor = glm::vec3(1.0f);
        UpdateAnimationRotation(animComp, rotComp, rotColor);

        RenderComponent* renderComp = entity->GetComponent<RenderComponent>();
        glm::vec3 finalColor = glm::vec3(1.0f);
        if (renderComp) // Update the color of the mesh
        {
            if (posColor != white) // Proritize position
            {
                finalColor = posColor;
            }
            else if (scaleColor != white)
            {
                finalColor = scaleColor;
            }
            else if (rotColor != white)
            {
                finalColor = rotColor;
            }

            renderComp->colorOverride = finalColor;
        }
    }
}

int AnimationLayer::FindKeyFramePositionIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFramePositions.size(); index++)
    {
        if (animComp->keyFramePositions[index].time > time) return index - 1;
    }

    return animComp->keyFramePositions.size() - 1;
}

int AnimationLayer::FindKeyFrameScaleIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFrameScales.size(); index++)
    {
        if (animComp->keyFrameScales[index].time > time) return index - 1;
    }

    return animComp->keyFrameScales.size() - 1;
}

int AnimationLayer::FindKeyFrameRotationIndex(AnimationComponent* animComp, float time)
{
    int index = 1;
    for (; index < animComp->keyFrameRotations.size(); index++)
    {
        if (animComp->keyFrameRotations[index].time > time) return index - 1;
    }

    return animComp->keyFrameRotations.size() - 1;
}

void AnimationLayer::UpdateAnimationPosition(AnimationComponent* animComp, PositionComponent* posComp, glm::vec3& color)
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
        color = red;
        break;
    case EaseOut:
        positionFraction = glm::sineEaseOut(positionFraction);
        color = yellow;
        break;
    case EaseInOut:
        positionFraction = glm::sineEaseInOut(positionFraction);
        color = green;
        break;
    default:
        color = white;
        break;
    }

    posComp->value = keyFramePos1.position + (keyFramePos2.position - keyFramePos1.position) * positionFraction;

    if (positionFraction > keyFramePos1.time && positionFraction < keyFramePos2.time)
    {
        KeyFrame frame;
        frame.time = positionFraction;
        frame.type = KeyFrameType::Position;
        frame.easingType = keyFramePos2.easingType;
        keyFrameListener->OnKeyFrame(frame);
    }
}

void AnimationLayer::UpdateAnimationScale(AnimationComponent* animComp, ScaleComponent* scaleComp, glm::vec3& color)
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
        color = red;
        break;
    case EaseOut:
        scaleFraction = glm::sineEaseOut(scaleFraction);
        color = yellow;
        break;
    case EaseInOut:
        scaleFraction = glm::sineEaseInOut(scaleFraction);
        color = green;
        break;
    default:
        color = white;
        break;
    }

    scaleComp->value = keyFrameScale1.scale + (keyFrameScale2.scale - keyFrameScale1.scale) * scaleFraction;


    if (scaleFraction > keyFrameScale1.time && scaleFraction < keyFrameScale2.time)
    {
        KeyFrame frame;
        frame.time = scaleFraction;
        frame.type = KeyFrameType::Scale;
        frame.easingType = keyFrameScale2.easingType;
        keyFrameListener->OnKeyFrame(frame);
    }
}

void AnimationLayer::UpdateAnimationRotation(AnimationComponent* animComp, RotationComponent* rotComp, glm::vec3& color)
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
        color = red;
        break;
    case EaseOut:
        rotFraction = glm::sineEaseOut(rotFraction);
        color = yellow;
        break;
    case EaseInOut:
        rotFraction = glm::sineEaseInOut(rotFraction);
        color = green;
        break;
    default:
        color = white;
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

    if (rotFraction > keyFrameRot1.time && rotFraction < keyFrameRot2.time)
    {
        KeyFrame frame;
        frame.time = rotFraction;
        frame.type = KeyFrameType::Rotation;
        frame.easingType = keyFrameRot2.easingType;
        keyFrameListener->OnKeyFrame(frame);
    }
}