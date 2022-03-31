#pragma once

#include "ApplicationLayer.h"
#include "SkeletalAnimationComponent.h"

#include <vector>

class AnimatedMesh;
class SkeletalAnimationLayer : public ApplicationLayer
{
public:
	struct AnimationData
	{
		AnimatedMesh* animatedMesh;
		SkeletalAnimationComponent* animationComp;
	};

	SkeletalAnimationLayer();
	virtual ~SkeletalAnimationLayer();

	virtual void OnUpdate(float deltaTime) override;

	std::vector<AnimationData> animations;

private:
	void ComputeBoneTransforms(SkeletalAnimationComponent* anim, const Bone& bone, const glm::mat4& inverseTransform, glm::mat4 parentTransform, bool root);
};