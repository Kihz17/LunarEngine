#include "SkeletalAnimationComponentListener.h"
#include "SkeletalAnimationComponent.h"

SkeletalAnimationComponentListener::SkeletalAnimationComponentListener(std::vector<SkeletalAnimationComponent*>& animations)
	: animations(animations)
{

}

SkeletalAnimationComponentListener::~SkeletalAnimationComponentListener()
{

}

void SkeletalAnimationComponentListener::OnAddComponent(Entity* entity, Component* component)
{
	SkeletalAnimationComponent* animComp = dynamic_cast<SkeletalAnimationComponent*>(component);
	if (!animComp) return;

	animations.push_back(animComp);
}

void SkeletalAnimationComponentListener::OnRemoveComponent(Entity* entity, Component* component)
{
	SkeletalAnimationComponent* animComp = dynamic_cast<SkeletalAnimationComponent*>(component);
	if (!animComp) return;

	int removeIndex = -1;
	for (int i = 0; i < animations.size(); i++)
	{
		if (animComp == animations[i])
		{
			removeIndex = i;
			break;
		}
	}

	if (removeIndex > -1)
	{
		animations.erase(animations.begin() + removeIndex);
	}
}