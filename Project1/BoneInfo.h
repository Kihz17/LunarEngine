#pragma once

#include <glm/glm.hpp>

struct BoneInfo
{
	unsigned int boneID;
	glm::mat4 bindTransform;
};