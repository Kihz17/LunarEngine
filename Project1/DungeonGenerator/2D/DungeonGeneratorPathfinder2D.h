#pragma once

#include "Grid2D.h"
#include "DungeonGeneratorTypes.h"
#include "Utils.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <functional>
#include <unordered_set>
#include <stack>
#include <queue>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class DungeonGeneratorPathfinder2D
{
public:
	struct Node
	{
		Node(const glm::ivec2& pos)
			: pos(pos),
			prev(nullptr),
			cost(std::numeric_limits<float>::max()),
			randCost(Utils::RandFloat(0.0f, 10.0f))
		{}

		glm::ivec2 pos;
		Node* prev;
		float cost;
		float randCost;
	};

	struct NodeCompare
	{
		bool operator()(const Node* n1, Node* n2)
		{
			return n1->cost > n2->cost;
		}
	};

	struct PathCost
	{
		bool traversable = false;
		float cost = std::numeric_limits<float>::max();
		bool isStairs = false;
	};

	DungeonGeneratorPathfinder2D(const glm::ivec2& size);
	~DungeonGeneratorPathfinder2D();

	std::vector<glm::ivec2> Pathfind(const glm::ivec2& start, const glm::ivec2& end, Grid2D<CellType>& dungeonGrid);
	void ResetNodes();
	std::vector<glm::ivec2> ReconstructPath(Node* node);

private:
	PathCost CostFunction(Node* n1, Node* n2, Grid2D<CellType>& dungeonGrid, const glm::ivec2 startPos, const glm::ivec2& endPos);
	Node* FindNextNode(std::priority_queue<Node*, std::vector<Node*>, NodeCompare>& queue, std::unordered_map<Node*, float>& map);

	Grid2D<Node*> grid;

	glm::ivec2 size;
};