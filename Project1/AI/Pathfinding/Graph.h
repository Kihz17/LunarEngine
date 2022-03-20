#pragma once

#include "Grid2D.h"

#include <string>
#include <vector>

class Graph
{
public:
	enum class NodeType
	{
		None,
		Difficult,
		Wall,
		StartPosition,
		Resource,
		Home
	};

	struct Node
	{
		Node* prev = nullptr;
		NodeType type = NodeType::None;
		bool visited = false;
		float cost = std::numeric_limits<float>::max();
		float hDistance = 0;
		std::vector<std::pair<Node*, float>> edges;
		glm::ivec2 pos;
	};

	Graph(const std::string& path);
	~Graph();

	void Reset();

	Grid2D<Node*>* GetGrid() { return grid; }

private:
	bool SetupEdge(const glm::ivec2& current, const glm::ivec2& neightbour, bool diag);

	int width, height, channels;
	Grid2D<Node*>* grid;
};