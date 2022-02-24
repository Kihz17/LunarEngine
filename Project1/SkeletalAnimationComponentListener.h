#pragma once

#include "IComponentListener.h"
#include "SkeletalAnimationLayer.h"

#include <vector>

class SkeletalAnimationComponentListener : public IComponentListener
{
public:
	SkeletalAnimationComponentListener(std::vector<SkeletalAnimationLayer::AnimationData>& animations);
	virtual ~SkeletalAnimationComponentListener();

	virtual void OnAddComponent(Entity* entity, Component* component) override;
	virtual void OnRemoveComponent(Entity* entity, Component* component) override;

private:
	std::vector<SkeletalAnimationLayer::AnimationData>& animations;
};