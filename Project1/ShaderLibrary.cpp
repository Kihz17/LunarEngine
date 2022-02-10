#include "ShaderLibrary.h"

#include <iostream>

std::unordered_map<std::string, Shader*> ShaderLibrary::shaders;
std::unordered_map<std::string, Shader*> ShaderLibrary::shadersByPath;

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

Shader* ShaderLibrary::Load(const std::string& name, const std::string& path)
{
	// Check if the shader has been loaded already
	{
		std::unordered_map<std::string, Shader*>::iterator it = shadersByPath.find(path);
		if (it != shadersByPath.end())
		{
			it->second;
		}
	}
	
	// Check if the name can be used
	{
		std::unordered_map<std::string, Shader*>::iterator it = shaders.find(name);
		if (it != shaders.end())
		{
			std::cout << "Shader with name '" << name << "' is already in use!\n";
			return nullptr;
		}
	}


	Shader* shader = new Shader(path);
	shaders.insert({ name, shader });
	shadersByPath.insert({ path, shader });
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