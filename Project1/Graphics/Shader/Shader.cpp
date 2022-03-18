#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

const unsigned int MAX_LINE_LENGTH = 65536;		// 16x1024

static std::vector<std::string> LoadSourceFromFile(const std::string& filepath)
{
	std::vector<std::string> source;
	std::ifstream theFile(filepath.c_str());
	if (!theFile.is_open())
	{
		return source;
	}

	char pLineTemp[MAX_LINE_LENGTH] = { 0 };
	while (theFile.getline(pLineTemp, MAX_LINE_LENGTH))
	{
		std::string tempString(pLineTemp);
		source.push_back(tempString);
	}

	theFile.close();
	return source;
}

bool Shader::WasThereACompileError(const GLuint& shaderID, const std::string& filePath)
{
	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		char* pLogText = new char[maxLength];
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, pLogText);

		std::stringstream ss;
		ss << filePath << ": Compile Error: " << pLogText;

		std::cout << ss.str() << std::endl;

		delete[] pLogText;

		return true;
	}

	return false;
}

static bool CompileShaderFromSource(const GLuint& shaderID, const std::string& filePath, const std::vector<std::string>& source)
{
	const unsigned int MAXLINESIZE = 8 * 1024;	// About 8K PER LINE, which seems excessive
	unsigned int numberOfLines = static_cast<unsigned int>(source.size());

	char** arraySource = new char* [numberOfLines];

	// Clear array to all zeros (i.e. '\0' or null terminator)
	memset(arraySource, 0, numberOfLines);

	for (unsigned int indexLine = 0; indexLine != numberOfLines; indexLine++)
	{
		unsigned int numCharacters = (unsigned int)source[indexLine].length();
		// Create an array of chars for each line
		arraySource[indexLine] = new char[numCharacters + 2];		// For the '\n' and '\0' at end
		memset(arraySource[indexLine], 0, numCharacters + 2);

		// Copy line of source into array
		for (unsigned int indexChar = 0; indexChar != source[indexLine].length(); indexChar++)
		{
			arraySource[indexLine][indexChar] = source[indexLine][indexChar];
		}

		// At a '\0' at end (just in case)
		arraySource[indexLine][numCharacters + 0] = '\n';
		arraySource[indexLine][numCharacters + 1] = '\0';
	}

	glShaderSource(shaderID, numberOfLines, arraySource, NULL);
	glCompileShader(shaderID);

	// Get rid of the temp source "c" style array
	for (unsigned int indexLine = 0; indexLine != numberOfLines; indexLine++)
	{
		delete[] arraySource[indexLine];
	}

	delete[] arraySource;

	return !Shader::WasThereACompileError(shaderID, filePath);
}

bool Shader::WasThereALinkError(const GLuint& programID)
{
	GLint wasError = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &wasError);
	if (wasError == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

		char* pLogText = new char[maxLength];
		glGetProgramInfoLog(programID, maxLength, &maxLength, pLogText);

		std::stringstream ss;
		ss << "Shader Link Error: " << pLogText;

		std::cout << ss.str() << std::endl;

		delete[] pLogText;
		return true;
	}

	return false;
}

Shader::Shader(const std::string& path)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint geometryShaderID = 0;
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	std::vector<std::string> vertexSource;
	std::vector<std::string> geometrySource;
	std::vector<std::string> fragmentSource;

	ShaderSourceType currentSourceType = ShaderSourceType::None;
	for (std::string line : LoadSourceFromFile(path))
	{
		if (line == "//type vertex")
		{
			currentSourceType = ShaderSourceType::Vertex;
		}
		else if (line == "//type fragment")
		{
			currentSourceType = ShaderSourceType::Fragment;
		}
		else if (line == "//type geometry")
		{
			currentSourceType = ShaderSourceType::Geometry;
		}

		if (currentSourceType == ShaderSourceType::Vertex)
		{
			vertexSource.push_back(line);
		}
		else if(currentSourceType == ShaderSourceType::Fragment)
		{
			fragmentSource.push_back(line);
		}
		else if (currentSourceType == ShaderSourceType::Geometry)
		{
			geometrySource.push_back(line);
		}
	}

	if (vertexSource.empty() || fragmentSource.empty())
	{
		std::cout << "[ERROR] Failed to find vertex and fragment source for shader: " << path << std::endl;
		return;
	}

	bool compiled = CompileShaderFromSource(vertexShaderID, path, vertexSource);
	if (!compiled)
	{
		std::cout << "[ERROR] Failed to compile Vertex Shader! Check log for details." << std::endl;
		return;
	}

	bool hasGeometryShader = !geometrySource.empty();
	if (hasGeometryShader)
	{
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		compiled = CompileShaderFromSource(geometryShaderID, path, geometrySource);
		if (!compiled)
		{
			std::cout << "[ERROR] Failed to compile Geometry Shader! Check log for details." << std::endl;
			return;
		}
	}

	compiled = CompileShaderFromSource(fragmentShaderID, path, fragmentSource);
	if (!compiled)
	{
		std::cout << "[ERROR] Failed to compile Fragment Shader! Check log for details." << std::endl;
		return;
	}

	this->ID = glCreateProgram();
	glAttachShader(ID, vertexShaderID);

	if (hasGeometryShader)
	{
		glAttachShader(ID, geometryShaderID);
	}

	glAttachShader(ID, fragmentShaderID);
	glLinkProgram(ID);

	size_t index = path.find_last_of('/');
	if (index == std::string::npos) // Not found, try blackslash
	{
		index = path.find_last_of('\\');
	}

	if (index == std::string::npos) // Not found, just assign to the path
	{
		fileName = path;
	}
	else
	{
		fileName = path.substr(index + 1);
	}

	WasThereALinkError(ID);
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

void Shader::InitializeUniform(const std::string& name)
{
	uniforms.insert({ name, glGetUniformLocation(ID, name.c_str()) });
}

void Shader::Bind() const
{
	glUseProgram(ID);
}

void Shader::Unbind() const
{
	glUseProgram(0);
}

GLuint Shader::GetUniformLocation(const std::string& name) const
{
	if (uniforms.count(name) <= 0)
	{
		std::cout << "Could not find uniform locaiton '" << name << "' in " << this->fileName << std::endl;
		return 0;
	}

	return uniforms.at(name);
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(GetUniformID(name), value);
}

void Shader::SetIntArray(const std::string& name, int* values, uint32_t count) const
{
	glUniform1iv(GetUniformID(name), count, values);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(GetUniformID(name), value);
}

void Shader::SetFloat2(const std::string& name, const glm::vec2& value) const
{
	glUniform2f(GetUniformID(name), value.x, value.y);
}

void Shader::SetFloat3(const std::string& name, const glm::vec3& value) const
{
	glUniform3f(GetUniformID(name), value.x, value.y, value.z);
}

void Shader::SetFloat4(const std::string& name, const glm::vec4& value) const
{
	glUniform4f(GetUniformID(name), value.x, value.y, value.z, value.w);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
	glUniformMatrix4fv(GetUniformID(name), 1, GL_FALSE, glm::value_ptr(value));
}

GLuint Shader::GetUniformID(const std::string& name) const
{
	if (uniforms.count(name) <= 0)
	{
		std::cout << "Uniform location '" << name << "' does not exist in shader " << fileName << std::endl;
		return 0;
	}

	return uniforms.at(name);
}