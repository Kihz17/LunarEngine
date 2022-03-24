#pragma once

#include "Utils.h"

#include <glm/glm.hpp>

struct TerrainGenerationInfo
{
	glm::vec2 seed = glm::vec2(Utils::RandFloat(0.0f, 1000.0f), Utils::RandFloat(0.0f, 1000.0f));
	float amplitude = 1.0f;
	float roughness = 0.3f;
	float persitence = 5.0f;
	float frequency = 0.001f;
	int octaves = 3;
};