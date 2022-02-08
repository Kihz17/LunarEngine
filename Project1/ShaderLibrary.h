#pragma once

#include "Shader.h"

class ShaderLibrary
{
public:
	static void CleanUp();

	static void Add(const std::string& name, Shader* shader);
	static Shader* Load(const std::string& name, const std::string& path);
	static Shader* Get(const std::string& name);
private:
	static std::unordered_map<std::string, Shader*> shaders;
};