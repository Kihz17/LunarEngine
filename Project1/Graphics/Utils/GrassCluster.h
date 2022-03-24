#pragma once

#include "VertexArrayObject.h"
#include "Texture2D.h"

#include <glm/glm.hpp>

struct GrassCluster
{
	VertexArrayObject* VAO;
	VertexBuffer* VBO;

	Texture2D* discardTexture = nullptr;
	Texture2D* albedoTexture = nullptr;
	Texture2D* normalTexture = nullptr;

	Texture2D* roughnessTexture = nullptr;
	Texture2D* metalTexture = nullptr;

	float roughness = 0.5f;
	float metalness = 0.01f;
	float ao = 1.0f;

	std::vector<glm::vec4> grassData; // xyz = root pos, w = rotation angle

	glm::vec2 windDirection = glm::vec2(0.3f, 0.9f);
	float oscillationStrength = 2.5f;
	float windForceMult = 1.0f;
	float stiffness = 0.8f;
	glm::vec2 dimensions = glm::vec2(0.1f, 0.2f);
};
