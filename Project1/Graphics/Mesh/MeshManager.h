#pragma once

#include "Mesh.h"
#include "AnimatedMesh.h"
#include "Animation.h"

#include <unordered_map>
#include <string>

class MeshManager
{
public:
	static void CleanUp();

	static Mesh* GetMesh(const std::string& path);
	static AnimatedMesh* GetAnimatedMesh(const std::string& path);
	static Animation* GetAnimation(const std::string& path);

private:
	static std::unordered_map<std::string, Mesh*> meshes;
	static std::unordered_map<std::string, AnimatedMesh*> animatedMeshes;
	static std::unordered_map<std::string, Animation*> animations;
};