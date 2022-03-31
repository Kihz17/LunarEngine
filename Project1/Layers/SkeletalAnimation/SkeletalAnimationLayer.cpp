#include "SkeletalAnimationLayer.h"
#include "Animation.h"
#include "AnimatedMesh.h"

#include <glm/gtx/matrix_interpolation.hpp>

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

		ComputeBoneTransforms(animComp, mesh->GetRootBone(), mesh->GetInverseTransform(), glm::mat4(1.0f), true);

		if (animComp->lerping)
		{
			animComp->lerpTime += animComp->lerpSpeed * deltaTime;
			if (animComp->lerpTime >= 1.0f) animComp->lerping = false;
		}
	}
}

void SkeletalAnimationLayer::ComputeBoneTransforms(SkeletalAnimationComponent* anim, const Bone& bone, const glm::mat4& inverseTransform, glm::mat4 parentTransform, bool root)
{
	Animation* animation = anim->anim;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
 	animation->GetFrameData(bone.name, anim->currentTime, position, rotation, scale);

	if (root && !anim->allowRootMotion) position = glm::vec3(0.0f, 0.0f, 0.0f); 

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

	if (anim->lerping)
	{
		glm::vec3 lastPosition;
		glm::quat lastRotation;
		glm::vec3 lastScale;
		anim->lastAnim->GetFrameData(bone.name, anim->lastTime, lastPosition, lastRotation, lastScale);

		if (root && !anim->allowRootMotion) lastPosition = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::quat rot = glm::slerp(lastRotation, rotation, anim->lerpTime);
		glm::vec3 pos = glm::mix(lastPosition, position, anim->lerpTime);
		glm::vec3 sc = glm::mix(lastScale, scale, anim->lerpTime);

		transform = glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), sc);
	}

	glm::mat4 globalTransform = parentTransform * transform;

	glm::mat4 transformResult = inverseTransform * globalTransform * bone.offsetTransform;

	anim->boneMatrices[bone.ID] = transformResult; // Assign the bone matrix at the bone's index

	for (const Bone& bone : bone.children) // Go through the bone's children and do the same thing
	{
		ComputeBoneTransforms(anim, bone, inverseTransform, globalTransform, false);
	}
}