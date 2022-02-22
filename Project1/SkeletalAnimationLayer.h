#pragma once

#include "ApplicationLayer.h"
#include "SkeletalAnimationComponent.h"

#include <vector>

class SkeletalAnimationLayer : public ApplicationLayer
{
public:
	SkeletalAnimationLayer();
	virtual ~SkeletalAnimationLayer();

	virtual void OnUpdate(float deltaTime) override;

	std::vector<SkeletalAnimationComponent*> animations;

private:
	void ComputeBoneTransforms(SkeletalAnimationComponent* component, const NodeData* node, glm::mat4 parentTransform);
};