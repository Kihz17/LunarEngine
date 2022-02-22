#pragma once

#include "IComponentListener.h"

#include <vector>

class SkeletalAnimationComponent;
class SkeletalAnimationComponentListener : public IComponentListener
{
public:
	SkeletalAnimationComponentListener(std::vector<SkeletalAnimationComponent*>& animations);
	virtual ~SkeletalAnimationComponentListener();

	virtual void OnAddComponent(Entity* entity, Component* component) override;
	virtual void OnRemoveComponent(Entity* entity, Component* component) override;

private:
	std::vector<SkeletalAnimationComponent*>& animations;
};