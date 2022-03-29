#include "Animation.h"
#include "IMesh.h"
#include "AnimatedMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <iostream>

const unsigned int Animation::MAX_BONES = 100;

Animation::Animation(const std::string& path)
	: filePath(path),
	duration(0),
	ticksPerSecond(0)
{
	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path, MeshUtils::ASSIMP_FLAGS);
	if (!assimpScene || !assimpScene->mRootNode)
	{
		std::cout << "Failed to load animation file: " << path << std::endl;
		return;
	}

	aiAnimation* assimpAnim = assimpScene->mAnimations[0];

	ticksPerSecond = assimpAnim->mTicksPerSecond;
	duration = assimpAnim->mDuration;

	for (unsigned int j = 0; j < assimpAnim->mNumChannels; j++)
	{
		aiNodeAnim* assimpNode = assimpAnim->mChannels[j];
		std::string boneName = assimpNode->mNodeName.C_Str();

		// Setup position key frames
		keyFrameMap[boneName].positions.resize(assimpNode->mNumPositionKeys);
		for (unsigned int i = 0; i < assimpNode->mNumPositionKeys; i++)
		{
			aiVector3D assimpPos = assimpNode->mPositionKeys[i].mValue;

			// Make new position Key Frame
			KeyFramePosition frame;
			frame.position = glm::vec3(assimpPos.x, assimpPos.y, assimpPos.z);
			frame.timeStamp = assimpNode->mPositionKeys[i].mTime;
			keyFrameMap[boneName].positions[i] = frame;
		}

		// Setup rotation key frames
		keyFrameMap[boneName].rotations.resize(assimpNode->mNumRotationKeys);
		for (unsigned int i = 0; i < assimpNode->mNumRotationKeys; i++)
		{
			aiQuaternion assimpRot = assimpNode->mRotationKeys[i].mValue;

			// Make new rotation Key Frame
			KeyFrameRotation frame;
			frame.rotation = glm::quat(assimpRot.w, assimpRot.x, assimpRot.y, assimpRot.z);
			frame.timeStamp = assimpNode->mRotationKeys[i].mTime;
			keyFrameMap[boneName].rotations[i] = frame;
		}

		// Setup scale key frames
		keyFrameMap[boneName].scales.resize(assimpNode->mNumScalingKeys);
		for (unsigned int i = 0; i < assimpNode->mNumScalingKeys; i++)
		{
			aiVector3D assimpScale = assimpNode->mScalingKeys[i].mValue;

			// Make new scale Key Frame
			KeyFrameScale frame;
			frame.scale = glm::vec3(assimpScale.x, assimpScale.y, assimpScale.z);
			frame.timeStamp = assimpNode->mScalingKeys[i].mTime;
			keyFrameMap[boneName].scales[i] = frame;
		}
	}
}

bool Animation::GetFrameData(const std::string& boneName, float time, glm::vec3& lerpedPos, glm::quat& lerpedRot, glm::vec3& lerpedScale)
{
	std::unordered_map<std::string, KeyFrames>::iterator it = keyFrameMap.find(boneName);
	if (it == keyFrameMap.end())
	{
		std::cout << "Bone '" << boneName << "' does not exist in animation " << filePath << ".\n";
		return false;
	}
		
	KeyFrames& keyFrames = it->second;

	if (keyFrames.positions.empty())
	{
		std::cout << "Bone '" << boneName << "' does not have any position key frames! File:" << filePath << ".\n";
		return false;
	}

	if (keyFrames.rotations.empty())
	{
		std::cout << "Bone '" << boneName << "' does not have any rotation key frames! File:" << filePath << ".\n";
		return false;
	}

	if (keyFrames.scales.empty())
	{
		std::cout << "Bone '" << boneName << "' does not have any scale key frames! File:" << filePath << ".\n";
		return false;
	}

	// Get positions
	KeyFramePosition positionFrame = keyFrames.positions[0];
	KeyFramePosition nextPositionFrame = keyFrames.positions[0];
	for (unsigned int i = 0; i < keyFrames.positions.size() - 1; i++)
	{
		int index = i + 1;
		if (time < keyFrames.positions[index].timeStamp)
		{
			positionFrame = keyFrames.positions[i];
			nextPositionFrame = keyFrames.positions[index];
			break;
		}
	}

	// Get rotations
	KeyFrameRotation rotationFrame = keyFrames.rotations[0];
	KeyFrameRotation nextRotationFrame = keyFrames.rotations[0];
	for (unsigned int i = 0; i < keyFrames.rotations.size() - 1; i++)
	{
		int index = i + 1;
		if (time < keyFrames.rotations[index].timeStamp)
		{
			rotationFrame = keyFrames.rotations[i];
			nextRotationFrame = keyFrames.rotations[index];
			break;
		}
	}

	// Get scales
	KeyFrameScale scaleFrame = keyFrames.scales[0];
	KeyFrameScale nextScaleFrame = keyFrames.scales[0];
	for (unsigned int i = 0; i < keyFrames.scales.size() - 1; i++)
	{
		int index = i + 1;
		if (time < keyFrames.scales[index].timeStamp)
		{
			scaleFrame = keyFrames.scales[i];
			nextScaleFrame = keyFrames.scales[index];
			break;
		}
	}

	float positionLerpTime = (time - positionFrame.timeStamp) / (nextPositionFrame.timeStamp - positionFrame.timeStamp);
	float rotationLerpTime = (time - rotationFrame.timeStamp) / (nextRotationFrame.timeStamp - rotationFrame.timeStamp);
	float scaleLerpTime = (time - scaleFrame.timeStamp) / (nextScaleFrame.timeStamp - scaleFrame.timeStamp);

	lerpedPos = glm::mix(positionFrame.position, nextPositionFrame.position, positionLerpTime);
	lerpedRot = glm::slerp(rotationFrame.rotation, nextRotationFrame.rotation, rotationLerpTime);
	lerpedScale = glm::mix(scaleFrame.scale, nextScaleFrame.scale, scaleLerpTime);

	return true;
}