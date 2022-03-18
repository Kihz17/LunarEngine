#pragma once

#include "DungeonGeneratorTypes.h"

#include <vector>

class Delaunay2D
{
public:
	void Triangulate();

	std::vector<glm::vec2> vertices;
	std::vector<DelaunayTriangle2D> triangles;
	std::vector<Edge2D> edges;
};