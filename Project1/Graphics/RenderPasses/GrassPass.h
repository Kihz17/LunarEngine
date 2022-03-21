#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"

class GrassPass
{
public:
	GrassPass(int maxGrassBlades);
	~GrassPass();

	void DoPass(const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view);

	void AddGrass(const std::vector<glm::vec3>& v);

	static const std::string GRASS_SHADER_KEY;

private:
	Shader* shader;
	VertexArrayObject* grassVAO;
	VertexBuffer* grassVBO;

	int maxGrassBlades;
	std::vector<glm::vec3> grassPositions;

	float oscillationStrength;
	float windForceMult;
	glm::vec2 windDirection;
};