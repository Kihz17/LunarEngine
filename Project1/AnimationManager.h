#pragma once

#include "Entity.h"

#include <unordered_map>

class AnimationComponent;
class PositionComponent;
class ScaleComponent;
class RotationComponent;
class AnimationManager
{
public:
	AnimationManager() = default;
	virtual ~AnimationManager() = default;

	void Update(const std::unordered_map<unsigned int, Entity*>& entities, float deltaTime);

private:
	int FindKeyFramePositionIndex(AnimationComponent* animComp, float time);
	int FindKeyFrameScaleIndex(AnimationComponent* animComp, float time);
	int FindKeyFrameRotationIndex(AnimationComponent* animComp, float time);

	void UpdateAnimationPosition(AnimationComponent* animComp, PositionComponent* posComp);
	void UpdateAnimationScale(AnimationComponent* animComp,ScaleComponent* posComp);
	void UpdateAnimationRotation(AnimationComponent* animComp, RotationComponent* posComp);
};