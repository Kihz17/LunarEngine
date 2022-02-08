#include "ShaderLibrary.h"

#include <iostream>

std::unordered_map<std::string, Shader*> ShaderLibrary::shaders;

void ShaderLibrary::CleanUp()
{
	std::unordered_map<std::string, Shader*>::iterator it = shaders.begin();
	while (it != shaders.end())
	{
		delete it->second;
		it++;
	}

	shaders.clear();
}

void ShaderLibrary::Add(const std::string& name, Shader* shader)
{
	std::unordered_map<std::string, Shader*>::iterator it = shaders.find(name);
	if (it != shaders.end())
	{
		std::cout << "Shader already exists with name '" << name << "'" << std::endl;
		return;
	}

	shaders.insert({ name, shader });
}

Shader* ShaderLibrary::Load(const std::string& name, const std::string& path)
{
	Shader* shader = new Shader(path);
	shaders.insert({ name, shader });
	return shader;
}

Shader* ShaderLibrary::Get(const std::string& name)
{
	std::unordered_map<std::string, Shader*>::iterator it = shaders.find(name);
	if (it == shaders.end())
	{
		return nullptr;
	}

	return it->second;
}