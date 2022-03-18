#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Path
{
	int currentNode;
	std::vector<glm::vec3> nodes;
	bool repeating;
	float arriveRadius;
	float pathRadius;
	int pathDirection;
};