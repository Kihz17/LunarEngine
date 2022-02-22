#include "Animation.h"

const unsigned int Animation::MAX_BONES = 100;

Animation::Animation(const std::string& path, AnimatedMesh* mesh)
{
	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path, MeshUtils::ASSIMP_FLAGS);
	if (!assimpScene || !assimpScene->mRootNode)
	{
		std::cout << "Failed to load animation file: " << path << std::endl;
		return;
	}

	aiAnimation* assimpAnim = assimpScene->mAnimations[0];
	duration = assimpAnim->mDuration;
	ticksPerSecond = assimpAnim->mTicksPerSecond;

	ParseHeirarchy(rootNode, assimpScene->mRootNode);
	CheckMissingBones(assimpAnim, mesh); // Sometimes  a mesh is missing bones for a specific animation, this ensures that the "new" bones found in the animation file are added
}

Bone* Animation::GetBone(const std::string& name)
{
	std::vector<Bone>::iterator it = std::find_if(bones.begin(), bones.end(), [&](const Bone& b) { return b.GetName() == name; });
	if (it == bones.end()) return nullptr;
	return &(*it);
}

void Animation::ParseHeirarchy(NodeData& loadTo, const aiNode* loadFrom)
{
	if (!loadFrom)
	{
		std::cout << "No assimp node to load animation heirarchy from!\n";
		return;
	}

	loadTo.name = loadFrom->mName.C_Str();
	loadTo.transform = MeshUtils::ConvertToGLMMat4(loadFrom->mTransformation);

	for (int i = 0; i < loadTo.children.size(); i++) // Create children recursivley
	{
		NodeData data;
		ParseHeirarchy(data, loadFrom->mChildren[i]);
		loadTo.children.push_back(data);
	}
}

void Animation::CheckMissingBones(const aiAnimation* assimpAnim, AnimatedMesh* mesh)
{
	std::map<std::string, BoneInfo>& meshBoneInfo = mesh->boneInfo;
	int& boneCount = mesh->boneCount;

	for (unsigned int i = 0; i < assimpAnim->mNumChannels; i++)
	{
		aiNodeAnim* assimpNode = assimpAnim->mChannels[i];
		std::string boneName = assimpNode->mNodeName.C_Str();

		if (meshBoneInfo.find(boneName) == meshBoneInfo.end()) // We found a bone that hasn't been accounted for!
		{
			meshBoneInfo[boneName].id = boneCount;
			boneCount++;
		}

		bones.push_back(Bone(boneName, meshBoneInfo[boneName].id, assimpNode)); // Add bone to the animations local bone storage
	}

	this->boneInfos = meshBoneInfo;
}