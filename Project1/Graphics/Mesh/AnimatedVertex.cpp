#include "AnimatedVertex.h"
#include <iostream>

AnimatedVertex::AnimatedVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord, int32_t* boneIDs, float* boneWeights)
{
	data[0] = position.x;
	data[1] = position.y;
	data[2] = position.z;

	data[3] = normal.x;
	data[4] = normal.y;
	data[5] = normal.z;

	data[6] = texCoord.x;
	data[7] = texCoord.y;

	SetBoneIDs(boneIDs);
	SetBoneWeights(boneWeights);
}

AnimatedVertex::~AnimatedVertex()
{

}

void AnimatedVertex::AddBoneData(int32_t boneID, float weight)
{
	int writeIndex = -1;
	for (unsigned int i = BONE_ID_START_INDEX; i < BONE_WEIGHT_START_INDEX; i++)
	{
		if (data[i] == -1)
		{
			writeIndex = i;
			break;
		}
	}

	if (writeIndex != -1)
	{
		data[writeIndex] = boneID; // Set boneID 
		data[writeIndex + MAX_BONE_INFLUENCE] = weight; // Set weight
	}
}

void AnimatedVertex::SetBoneIDs(int32_t* boneIDs)
{
	constexpr unsigned int boneIDSize = 8 + MAX_BONE_INFLUENCE;
	for (unsigned int i = 8; i < boneIDSize; i++) data[i] = boneIDs[i - 8];
}

void AnimatedVertex::SetBoneWeights(float* boneWeights)
{
	constexpr unsigned int boneWeightStart = 8 + MAX_BONE_INFLUENCE;
	constexpr unsigned int boneWeightSize = boneWeightStart + MAX_BONE_INFLUENCE;
	for (unsigned int i = boneWeightStart; i < boneWeightSize; i++) data[i] = boneWeights[i - boneWeightStart];
}