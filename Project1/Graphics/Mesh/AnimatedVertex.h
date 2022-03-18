#pragma once

#include "IVertex.h"

static const unsigned int MAX_BONE_INFLUENCE = 4; // WARNING: WHEN CHANGING THIS VALUE, MAKE SURE THAT THE BUFFER LAYOUT REFLECTS THE CHANGES (AnimatedMesh.cpp & in animated shader)
static const unsigned int BONE_ID_START_INDEX = 8; // Bone IDs start at index 8
static const unsigned int BONE_WEIGHT_START_INDEX = 8 + MAX_BONE_INFLUENCE; // Bone weights start at index 16

class AnimatedVertex : public IVertex
{
public:
	AnimatedVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord, int32_t* boneIDs, float* boneWeights);
	virtual ~AnimatedVertex();

	virtual float* Data() override { return data; }

	void AddBoneData(int32_t boneID, float weight);
	void SetBoneIDs(int32_t* boneIDs);
	void SetBoneWeights(float* boneWeights);

	static unsigned int Length() { return 8 + MAX_BONE_INFLUENCE * 2; }
	static unsigned int Size() { return (sizeof(float) * 8) + (sizeof(int32_t) * MAX_BONE_INFLUENCE) + (sizeof(float) * MAX_BONE_INFLUENCE); } // 8 floats (position, normal, texcoords), 4 int32_t (4 bone IDs), 4 floats (4 bone weights)

private:
	float data[8 + MAX_BONE_INFLUENCE * 2];
};