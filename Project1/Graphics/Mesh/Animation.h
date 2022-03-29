#pragma once

#include "BoneInfo.h"
#include "Bone.h"
#include "AnimatedVertex.h"

#include <assimp/scene.h>
#include <assimp/anim.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <unordered_map>

struct KeyFramePosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyFrameRotation
{
	glm::quat rotation;
	float timeStamp;
};

struct KeyFrameScale
{
	glm::vec3 scale;
	float timeStamp;
};

struct KeyFrames
{
	std::vector<KeyFramePosition> positions;
	std::vector<KeyFrameRotation> rotations;
	std::vector<KeyFrameScale> scales;
};

class AnimatedMesh;
class Animation
{
public:
	Animation(const std::string& path);

	const float& GetDuration() const { return duration; }
	const int& GetTicksPerSecond() const { return ticksPerSecond; }

	bool GetFrameData(const std::string& boneName, float time, glm::vec3& lerpedPos, glm::quat& lerpedRot, glm::vec3& lerpedScale);

	static const unsigned int MAX_BONES;

private:
	std::unordered_map<std::string, KeyFrames> keyFrameMap;

	float duration;
	int ticksPerSecond;

	std::string filePath;
};