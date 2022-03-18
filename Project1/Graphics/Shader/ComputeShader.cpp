#include "ComputeShader.h"
#include <fstream>
#include <sstream>

static std::string LoadSourceFromFile(const std::string& path)
{
	std::string src;
	std::ifstream ifs;

	ifs.open(path);

	std::stringstream shaderStream;
	shaderStream << ifs.rdbuf();

	ifs.close();

	src = shaderStream.str();

	return src;
}

ComputeShader::ComputeShader(const std::string& path)
{
	std::string src = LoadSourceFromFile(path);
	const char* cSrc = src.c_str();

	// Compile
	GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shaderID, 1, &cSrc, nullptr);
	glCompileShader(shaderID);
	if (Shader::WasThereACompileError(shaderID, path)) return;

	ID = glCreateProgram();
	glAttachShader(ID, shaderID);

	// Link
	glLinkProgram(ID);
	if (Shader::WasThereALinkError(ID)) return;

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
}