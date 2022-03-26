#include "MeshManager.h"

std::unordered_map<std::string, Mesh*> MeshManager::meshes;
std::unordered_map<std::string, AnimatedMesh*> MeshManager::animatedMeshes;

void MeshManager::CleanUp()
{
	std::unordered_map<std::string, Mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
	{
		delete it->second;
		it++;
	}
	meshes.clear();

	std::unordered_map<std::string, AnimatedMesh*>::iterator ait = animatedMeshes.begin();
	while (ait != animatedMeshes.end())
	{
		delete ait->second;
		ait++;
	}
	animatedMeshes.clear();
}

Mesh* MeshManager::GetMesh(const std::string& path)
{
	std::unordered_map<std::string, Mesh*>::iterator it = meshes.find(path);
	if (it != meshes.end()) return it->second;

	Mesh* m = new Mesh(path);
	meshes.insert({ path, m });
	return m;
}

AnimatedMesh* MeshManager::GetAnimatedMesh(const std::string& path)
{
	std::unordered_map<std::string, AnimatedMesh*>::iterator it = animatedMeshes.find(path);
	if (it != animatedMeshes.end()) return it->second;

	AnimatedMesh* m = new AnimatedMesh(path);
	animatedMeshes.insert({ path, m });
	return m;
}