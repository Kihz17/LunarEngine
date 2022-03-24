#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"
#include "GrassCluster.h"

class GrassPass
{
public:
	GrassPass(int maxGrassBlades);
	~GrassPass();

	void DoPass(IFrameBuffer* geometryBuffer, GrassCluster& grassCluster, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view);

	void AddGrass(const std::vector<glm::vec4>& v);

	static const std::string GRASS_SHADER_KEY;

private:
	Shader* shader;
	VertexArrayObject* grassVAO;
	VertexBuffer* grassVBO;

	int maxGrassBlades;
	std::vector<glm::vec4> grassPositions;

	float oscillationStrength;
	float windForceMult;
	float stiffness;
	glm::vec2 windDirection;

	Texture2D* grassDiscard;
	Texture2D* grassColor;
};