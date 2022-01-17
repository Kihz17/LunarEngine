#pragma once

#include "GLCommon.h"

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

enum class ShaderSourceType
{
	None,
	Vertex,
	Geometry,
	Fragment
};

class Shader
{
public:
	Shader(const std::string& path);
	virtual ~Shader();

	void InitializeUniform(const std::string& name);

	void Bind() const;
	void Unbind() const;

	const std::string& GetFileName() const {return fileName; }

	GLuint GetUniformLocation(const std::string& name) const;

	void SetInt(const std::string& name, int value) const;
	void SetIntArray(const std::string& name, int* values, uint32_t count) const;
	void SetFloat(const std::string& name, float value) const;
	void SetFloat2(const std::string& name, const glm::vec2& value) const;
	void SetFloat3(const std::string& name, const glm::vec3& value) const;
	void SetFloat4(const std::string& name, const glm::vec4& value) const;
	void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
	GLuint GetUniformID(const std::string& name) const;

	GLuint ID;
	std::string fileName;
	std::unordered_map<std::string, GLuint> uniforms;
};