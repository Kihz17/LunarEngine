#pragma once

#include "pch.h"
#include "Grid3D.h"
#include "DungeonGeneratorTypes.h"

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

struct NodePriorityFunction
{
	bool operator()(const float compareTo, const float value) const
	{
		return value < compareTo;
	}
};

class DungeonGeneratorPathfinder3D
{
public:
	struct Node
	{
		Node(const glm::ivec3& pos)
			: pos(pos),
			prev(nullptr),
			cost(std::numeric_limits<float>::max())
		{}

		std::ostream& operator<<(std::ostream& os)
		{
			os << "X: " << pos.x << " Y:" << pos.y << " Z:" << pos.z << "\Cost: " << cost << "\n";
		}

		glm::ivec3 pos;
		Node* prev;
		std::unordered_set<glm::ivec3> prevPath;
		float cost;
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

	DungeonGeneratorPathfinder3D(const glm::ivec3& size);
	~DungeonGeneratorPathfinder3D();

	std::vector<glm::ivec3> Pathfind(const glm::ivec3& start, const glm::ivec3& end, Grid3D<CellType>& dungeonGrid);
	void ResetNodes();
	std::vector<glm::ivec3> ReconstructPath(Node* node);

private:
	PathCost CostFunction(Node* n1, Node* n2, Grid3D<CellType>& dungeonGrid, const glm::ivec3 startPos, const glm::ivec3& endPos);
	Node* FindNextNode(std::priority_queue<Node*, std::vector<Node*>, NodeCompare>& queue, std::unordered_map<Node*, float>& map);

	Grid3D<Node*> grid;

	glm::ivec3 size;
};