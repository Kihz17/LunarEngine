#pragma once

#include "IMesh.h"

#include <assimp/scene.h>

#include <map>

class MeshManager
{
public:
	static IMesh* LoadMesh(const std::string& filePath, aiMesh* assimpMesh, unsigned int assimpMeshIndex);

private:
	static std::map<std::string, std::map<unsigned int, IMesh*>> loadedMeshes;
};