#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"

class TerrainPass
{
public:
	TerrainPass();

	void DoPass(const glm::mat4& projection, const glm::mat4& view);

	static const std::string TERRAIN_SHADER_KEY;
private:
	Texture2D* heightMap;
	VertexArrayObject* vao;
	VertexBuffer* vbo;

	Shader* shader;
};