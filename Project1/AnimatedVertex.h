#pragma once

#include "IVertex.h"


static const unsigned int MAX_BONE_INFLUENCE = 8;
static const unsigned int BONE_ID_START_INDEX = 8; // Bone IDs start at index 8
static const unsigned int BONE_WEIGHT_START_INDEX = 16; // Bone weights start at index 16

struct BoneInfo
{
	int id;
	glm::mat4 offset;
};

class AnimatedVertex : public IVertex
{
public:
	AnimatedVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord, int* boneIDs, float* boneWeights);
	virtual ~AnimatedVertex();

	virtual float* Data() override { return data; }

	void SetBoneIDs(int* boneIDs);
	void SetBoneWeights(float* boneWeights);

	static unsigned int Length() { return 24; }
	static unsigned int Size() { return (sizeof(float) * 6) + (sizeof(int) * MAX_BONE_INFLUENCE) + (sizeof(float) * MAX_BONE_INFLUENCE); } // 6 floats (position, normal, texcoords), 8 ints (8 bone IDs), 8 floats (8 bone weights)

private:
	float data[MAX_BONE_INFLUENCE];
};