#pragma once

#include "DungeonGeneratorTypes.h"

#include <glm/glm.hpp>

#include <vector>

struct Delaunay3D
{
	void Tetrahedralize();

	std::vector<glm::vec3> vertices;
	std::vector<Tetrahedron> tetrahedra;
	std::vector<DelaunayTriangle> triangles;
	std::vector<Edge> edges;
};