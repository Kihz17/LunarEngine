#include "SkeletalAnimationComponentListener.h"
#include "SkeletalAnimationComponent.h"
#include "RenderComponent.h"
#include "Entity.h"
#include "AnimatedMesh.h"

SkeletalAnimationComponentListener::SkeletalAnimationComponentListener(std::vector<SkeletalAnimationLayer::AnimationData>& animations)
	: animations(animations)
{

}

SkeletalAnimationComponentListener::~SkeletalAnimationComponentListener()
{

}

void SkeletalAnimationComponentListener::OnAddComponent(Entity* entity, Component* component)
{
	SkeletalAnimationComponent* animComp = dynamic_cast<SkeletalAnimationComponent*>(component);
	RenderComponent* renderComponent = dynamic_cast<RenderComponent*>(component);
	if (animComp) // We added an animation component
	{
		RenderComponent* renderComp = entity->GetComponent<RenderComponent>();
		if (!renderComp) return; // We added animation comp with no render, no need to do anything

		AnimatedMesh* riggedMesh = dynamic_cast<AnimatedMesh*>(renderComp->mesh);
		if (!riggedMesh) return; // The mesh on this entity is NOT rigged so we can't animate it

		animations.push_back({ riggedMesh, animComp });
	}
	else if (renderComponent) // We just added a render component
	{
		AnimatedMesh* riggedMesh = dynamic_cast<AnimatedMesh*>(renderComponent->mesh);
		if (!riggedMesh) return; // The mesh on this entity is NOT rigged so we can't animate it

		SkeletalAnimationComponent* animationComponent = entity->GetComponent<SkeletalAnimationComponent>();
		if (!animationComponent) return; // No animation component

		animations.push_back({ riggedMesh, animationComponent });
	}
	
}

void SkeletalAnimationComponentListener::OnRemoveComponent(Entity* entity, Component* component)
{
	SkeletalAnimationComponent* animComp = dynamic_cast<SkeletalAnimationComponent*>(component);
	RenderComponent* renderComp = dynamic_cast<RenderComponent*>(component);

	bool shouldRemove = animComp || (renderComp && dynamic_cast<AnimatedMesh*>(renderComp->mesh));
	if (!shouldRemove) return;

	int removeIndex = -1;
	for (int i = 0; i < animations.size(); i++)
	{
		if (animComp == animations[i].animationComp && animations[i].animatedMesh == renderComp->mesh)
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