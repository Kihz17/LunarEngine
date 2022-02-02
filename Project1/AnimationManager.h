#pragma once

#include "Entity.h"
#include "IKeyFrameListener.h"

#include <glm/glm.hpp>
#include <unordered_map>

class AnimationComponent;
class PositionComponent;
class ScaleComponent;
class RotationComponent;
class AnimationManager
{
public:
	AnimationManager();
	virtual ~AnimationManager();

	void Update(const std::unordered_map<unsigned int, Entity*>& entities, float deltaTime);
	
	void TogglePaused() { paused = !paused; }

private:
	int FindKeyFramePositionIndex(AnimationComponent* animComp, float time);
	int FindKeyFrameScaleIndex(AnimationComponent* animComp, float time);
	int FindKeyFrameRotationIndex(AnimationComponent* animComp, float time);

	void UpdateAnimationPosition(AnimationComponent* animComp, PositionComponent* posComp, glm::vec3& color);
	void UpdateAnimationScale(AnimationComponent* animComp,ScaleComponent* posComp, glm::vec3& color);
	void UpdateAnimationRotation(AnimationComponent* animComp, RotationComponent* posComp, glm::vec3& color);

	bool paused;

	IKeyFrameListener* keyFrameListener;
};