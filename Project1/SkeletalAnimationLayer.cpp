#include "SkeletalAnimationLayer.h"
#include "Animation.h"
#include "AnimatedMesh.h"

#include <iostream>

SkeletalAnimationLayer::SkeletalAnimationLayer()
{

}

SkeletalAnimationLayer::~SkeletalAnimationLayer()
{

}

void SkeletalAnimationLayer::OnUpdate(float deltaTime)
{
	for (AnimationData& animData : animations)
	{
		SkeletalAnimationComponent* animComp = animData.animationComp;
		Animation* animation = animComp->anim;
		AnimatedMesh* mesh = animData.animatedMesh;

		if (!animation) continue; // No animation currently tied to the component

		if (animComp->currentTime > animation->GetDuration() && !animComp->repeat) continue; // Not repeating

		// Advance the animation by ticks per second
		animComp->currentTime += animation->GetTicksPerSecond() * animComp->speed * deltaTime;
		animComp->currentTime = fmod(animComp->currentTime, animation->GetDuration());

		ComputeBoneTransforms(animComp, mesh->GetRootBone(), mesh->GetInverseTransform(), glm::mat4(1.0f));
	}
}

void SkeletalAnimationLayer::ComputeBoneTransforms(SkeletalAnimationComponent* anim, const Bone& bone, const glm::mat4& inverseTransform, glm::mat4 parentTransform)
{
	Animation* animation = anim->anim;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
 	animation->GetFrameData(bone.name, anim->currentTime, position, rotation, scale);

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 globalTransform = parentTransform * transform;

	glm::mat4 transformResult = inverseTransform * globalTransform * bone.offsetTransform;

	anim->boneMatrices[bone.ID] = transformResult; // Assign teh bone matrix at the bone's index

	for (const Bone& bone : bone.children) // Go through the bone's children and do the same thing
	{
		ComputeBoneTransforms(anim, bone, inverseTransform, globalTransform);
	}
}