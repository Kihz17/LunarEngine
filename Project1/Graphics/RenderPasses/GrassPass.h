#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"

struct GrassCluster
{
	VertexArrayObject* VAO;
	VertexBuffer* VBO;

	Texture2D* discardTexture;
	Texture2D* albedoTexture;
	Texture2D* normalTexture = nullptr;

	Texture2D* roughnessTexture = nullptr;
	Texture2D* metalTexture = nullptr;

	float roughness = 0.5f;
	float metalness = 0.01f;
	float ao = 1.0f;

	std::vector<glm::vec4> grassData; // xyz = root pos, w = rotation angle

	float oscillationStrength = 2.5f;
	float windForceMult = 1.0f;
	float stiffness = 0.8f;
	glm::vec2 dimensions = glm::vec2(0.1f, 0.2f);
};

class GrassPass
{
public:
	GrassPass(int maxGrassBlades);
	~GrassPass();

	void DoPass(IFrameBuffer* geometryBuffer, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view);

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