#pragma once

#include "DungeonGeneratorTypes.h"

#include <glm/glm.hpp>

#include <vector>

namespace DungeonGenUtils
{
	bool AlmostEqual(const glm::vec3& v1, const glm::vec3& v2);
	bool AlmostEqual(const glm::vec2& v1, const glm::vec2& v2);
	std::vector<Edge> MinimumSpanningTree(const std::vector<Edge>& edges, const glm::vec3& startVertex);
	std::vector<Edge2D> MinimumSpanningTree2D(const std::vector<Edge2D>& edges, const glm::vec2& startVertex);
}