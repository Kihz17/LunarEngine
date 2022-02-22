#pragma once

#include "Bone.h"
#include "AnimatedVertex.h"
#include "AnimatedMesh.h"

#include <map>

struct NodeData
{
	std::string name;
	glm::mat4 transform;
	std::vector<NodeData> children;
};

class Animation
{
public:
	Animation(const std::string& path, AnimatedMesh* mesh);

	Bone* GetBone(const std::string& name);

	float duration;
	int ticksPerSecond;
	NodeData rootNode;
	std::map<std::string, BoneInfo> boneInfos;

	static const unsigned int MAX_BONES;

private:
	void ParseHeirarchy(NodeData& loadTo, const aiNode* loadFrom);
	void CheckMissingBones(const aiAnimation* assimpAnim, AnimatedMesh* mesh);

	std::vector<Bone> bones;
};