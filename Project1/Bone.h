#pragma once

#include <vector>
#include <string>

#include <assimp/scene.h>

#include<glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

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

class Bone
{
public:
	Bone(const std::string& name, unsigned int ID, aiNodeAnim* channel);

	const int& GetID() const { return ID; }
	const std::string& GetName() const { return name; }

	void Update(float time);

	const glm::mat4& GetLocalTransform() const { return localTransform; }

private:
	glm::mat4 InterpolatePosition(float time);
	glm::mat4 InterpolateScale(float time);
	glm::mat4 InterpolateRotation(float time);

	int FindPositionIndexAtTime(float time);
	int FindRotationIndexAtTime(float time);
	int FindScaleIndexAtTime(float time);

	float ComputeLerpTime(const float lastTime, const float nextTime, const float currentTime);

	unsigned int ID;
	std::string name;

	glm::mat4 localTransform;

	std::vector<KeyFramePosition> positions;
	std::vector<KeyFrameRotation> rotations;
	std::vector<KeyFrameScale> scales;
};