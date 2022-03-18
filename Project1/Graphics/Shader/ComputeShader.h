#pragma once

#include "Shader.h"

class ComputeShader : public Shader
{
public:

private:
	friend class ShaderLibrary;
	ComputeShader(const std::string& path);
};