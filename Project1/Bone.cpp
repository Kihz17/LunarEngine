#include "Bone.h"

#include <iostream>

Bone::Bone(const std::string& name, unsigned int ID, aiNodeAnim* channel)
	: name(name),
	ID(ID),
	localTransform(1.0f)
{
	// Setup position key frames
	positions.resize(channel->mNumPositionKeys);
	for (unsigned int i = 0; i < channel->mNumPositionKeys; i++)
	{
		aiVector3D assimpPos = channel->mPositionKeys[i].mValue;

		// Make new position Key Frame
		KeyFramePosition frame;
		frame.position = glm::vec3(assimpPos.x, assimpPos.y, assimpPos.z);
		frame.timeStamp = channel->mPositionKeys[i].mTime;
		positions[i] = frame;
	}

	// Setup rotation key frames
	rotations.resize(channel->mNumRotationKeys);
	for (unsigned int i = 0; i < channel->mNumRotationKeys; i++)
	{
		aiQuaternion assimpRot = channel->mRotationKeys[i].mValue;

		// Make new rotation Key Frame
		KeyFrameRotation frame;
		frame.rotation = glm::quat(assimpRot.w, assimpRot.x, assimpRot.y, assimpRot.z);
		frame.timeStamp = channel->mRotationKeys[i].mTime;
		rotations[i] = frame;
	}

	// Setup scale key frames
	scales.resize(channel->mNumScalingKeys);
	for (unsigned int i = 0; i < channel->mNumScalingKeys; i++)
	{
		aiVector3D assimpScale = channel->mScalingKeys[i].mValue;

		// Make new rotation Key Frame
		KeyFrameScale frame;
		frame.scale = glm::vec3(assimpScale.x, assimpScale.y, assimpScale.z);
		frame.timeStamp = channel->mScalingKeys[i].mTime;
		scales[i] = frame;
	}
}

void Bone::Update(float time)
{
	localTransform = InterpolatePosition(time) * InterpolateRotation(time) * InterpolateScale(time);
}

glm::mat4 Bone::InterpolatePosition(float time)
{
	if (positions.size() == 1)
	{
		return glm::translate(glm::mat4(1.0f), positions[0].position);
	}

	// Get the frame for now and after
	int currentIndex = FindPositionIndexAtTime(time);
	int nextIndex = currentIndex + 1;

	float lerpTime = ComputeLerpTime(positions[currentIndex].timeStamp, positions[nextIndex].timeStamp, time); // Find time between now and next frame
	glm::vec3 pos = glm::mix(positions[currentIndex].position, positions[nextIndex].position, lerpTime); // Compute result

	return glm::translate(glm::mat4(1.0f), pos);
}

glm::mat4 Bone::InterpolateScale(float time)
{
	if (scales.size() == 1)
	{
		return glm::scale(glm::mat4(1.0f), scales[0].scale);
	}

	// Get the frame for now and after
	int currentIndex = FindScaleIndexAtTime(time);
	int nextIndex = currentIndex + 1;

	float lerpTime = ComputeLerpTime(scales[currentIndex].timeStamp, scales[nextIndex].timeStamp, time); // Find time between now and next frame
	glm::vec3 scale = glm::mix(scales[currentIndex].scale, scales[nextIndex].scale, lerpTime); // Compute result

	return glm::scale(glm::mat4(1.0f), scale);
}

glm::mat4 Bone::InterpolateRotation(float time)
{
	if (rotations.size() == 1)
	{
		return glm::toMat4(glm::normalize(rotations[0].rotation));
	}

	// Get the frame for now and after
	int currentIndex = FindRotationIndexAtTime(time);
	int nextIndex = currentIndex + 1;

	float lerpTime = ComputeLerpTime(rotations[currentIndex].timeStamp, rotations[nextIndex].timeStamp, time); // Find time between now and next frame
	glm::quat rot = glm::slerp(rotations[currentIndex].rotation, rotations[nextIndex].rotation, lerpTime); // Compute result

	return glm::toMat4(rot);
}

int Bone::FindPositionIndexAtTime(float time)
{
	for (unsigned int i = 0; i < positions.size() - 1; i++)
	{
		if (time < positions[i + 1].timeStamp) return i;
	}

	std::cout << "Failed to find position index\n";
	return 0;
}

int Bone::FindRotationIndexAtTime(float time)
{
	for (unsigned int i = 0; i < rotations.size() - 1; i++)
	{
		if (time < rotations[i + 1].timeStamp) return i;
	}

	std::cout << "Failed to find rotation index\n";
	return 0;
}

int Bone::FindScaleIndexAtTime(float time)
{
	for (unsigned int i = 0; i < scales.size() - 1; i++)
	{
		if (time < scales[i + 1].timeStamp) return i;
	}

	std::cout << "Failed to find scale index\n";
	return 0;
}

float Bone::ComputeLerpTime(const float lastTime, const float nextTime, const float currentTime)
{
	float halfway = currentTime - lastTime;
	float frameDiff = nextTime - lastTime;
	return halfway / frameDiff;
}