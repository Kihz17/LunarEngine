#include "SkeletalAnimationLayer.h"
#include "Animation.h"

SkeletalAnimationLayer::SkeletalAnimationLayer()
{

}

SkeletalAnimationLayer::~SkeletalAnimationLayer()
{

}

void SkeletalAnimationLayer::OnUpdate(float deltaTime)
{
	for (SkeletalAnimationComponent* anim : animations)
	{
		anim->deltaTime = deltaTime;

		Animation* animation = anim->anim;
		if (!animation) continue; // No animation currently tied to the component

		// Advance the animation by ticks per second
		anim->currentTime += animation->ticksPerSecond * deltaTime;
		anim->currentTime = fmod(anim->currentTime, animation->duration);

		ComputeBoneTransforms(anim, &animation->rootNode, glm::mat4(1.0f)); 
	}
}

void SkeletalAnimationLayer::ComputeBoneTransforms(SkeletalAnimationComponent* component, const NodeData* node, glm::mat4 parentTransform)
{
	Animation* animation = component->anim;
	std::string name = node->name;
	glm::mat4 transform = node->transform;

	Bone* bone = animation->GetBone(name); // Find bone for this node

	if(bone) // Update bone transform
	{
		bone->Update(component->currentTime);
		transform = bone->GetLocalTransform();
	}

	glm::mat4 globalTransform = parentTransform * transform; // Transform bone by its parent

	std::map<std::string, BoneInfo>& boneInfo = animation->boneInfos;
	std::map<std::string, BoneInfo>::iterator it = boneInfo.find(name);
	if (it != boneInfo.end())
	{
		int index = it->second.id;
		glm::mat4 offset = it->second.offset;
		component->boneMatrices[index] = globalTransform * offset; // Finally update the transformation that our shader will use
	}

	for (int i = 0; i < node->children.size(); i++) // Do the same for all child nodes
	{
		ComputeBoneTransforms(component, node, globalTransform);
	}
}