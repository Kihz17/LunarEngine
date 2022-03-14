#pragma once

#include "ComputeShader.h"

class ShaderLibrary
{
public:
	static void CleanUp();

	static ComputeShader* LoadCompute(const std::string& name, const std::string& path);
	static Shader* Load(const std::string& name, const std::string& path);
	static Shader* Get(const std::string& name);

private:
	static std::unordered_map<std::string, Shader*> shaders;
	static std::unordered_map<std::string, Shader*> shadersByPath;
};