#include "DungeonGeneratorPathfinder.h"

#include <iostream>

constexpr glm::ivec3 neighbours[] = // Represents the directions we can move in on the grid
{
	glm::ivec3(1, 0, 0),
	glm::ivec3(-1, 0, 0),

	glm::ivec3(0, 0, 1),
	glm::ivec3(0, 0, -1),

	glm::ivec3(0, 1, 0),
	glm::ivec3(0, -1, 0),
};

DungeonGeneratorPathfinder::DungeonGeneratorPathfinder(const glm::ivec3& size)
	: grid(size, glm::ivec3(0, 0, 0)),
	size(size)
{
	// Fill grid with nodes
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			for (int z = 0; z < size.z; z++)
			{
				grid.Set(glm::ivec3(x, y, z), new Node(glm::ivec3(x, y, z)));
			}
		}
	}
}

DungeonGeneratorPathfinder::~DungeonGeneratorPathfinder()
{
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			for (int z = 0; z < size.z; z++)
			{
				delete grid.Get(glm::ivec3(x, y, z));
			}
		}
	}
}

std::vector<glm::ivec3> DungeonGeneratorPathfinder::Pathfind(const glm::ivec3& start, const glm::ivec3& end, DungeonGrid<CellType>& dungeonGrid)
{
	ResetNodes();
	std::priority_queue<Node*, std::vector<Node*>, NodeCompare> queue;
	std::unordered_map<Node*, float> updatedQueueValues;
	std::unordered_set<Node*> closedSet;

	// Start us off at the start position
	if (!grid.InBounds(start)) std::cout << "Start not in bounds!\n";
	grid.Get(start)->cost = 0.0f;
	queue.push(grid.Get(start));

	while (!queue.empty())
	{
		Node* fromNode = FindNextNode(queue, updatedQueueValues); // Find the next best node
		if (fromNode->pos == end)
		{
			return ReconstructPath(fromNode);
		}

		for (const glm::ivec3& n : neighbours)
		{
			glm::ivec3 currentPos = fromNode->pos + n;
			if (!grid.InBounds(currentPos)) continue; // This position isn't in our grid

			Node* currentNode = grid.Get(currentPos);
			if (closedSet.find(currentNode) != closedSet.end()) continue; // We have already traversed here, don't re-caculate path cost

			PathCost pathCost = CostFunction(fromNode, currentNode, dungeonGrid, start, end);
			if (!pathCost.traversable) continue; // Can't go here

			// Update the node's cost and put it in the queue
			currentNode->cost = pathCost.cost;
			queue.push(currentNode);
			updatedQueueValues.insert({ currentNode, pathCost.cost });

			// Make sure we store that path leading up to this node
			for (const glm::ivec3& p : fromNode->prevPath) // Add all previous paths from
			{
				currentNode->prevPath.insert(p);
			}
			currentNode->prevPath.insert(fromNode->pos); // We came from this node
		}
	}
	
	return std::vector<glm::ivec3>(); // Couldn't form path
}

void DungeonGeneratorPathfinder::ResetNodes()
{
	glm::ivec3 gridSize = grid.Size();

	for (int x = 0; x < gridSize.x; x++)
	{
		for (int y = 0; y < gridSize.y; y++)
		{
			for (int z = 0; z < gridSize.z; z++)
			{
				Node*& node = grid.Get(glm::ivec3(x, y, z));
				node->prev = nullptr;
				node->cost = std::numeric_limits<float>::max();
				node->prevPath.clear();
			}
		}
	}
}

std::vector<glm::ivec3> DungeonGeneratorPathfinder::ReconstructPath(Node* node)
{
	std::vector<glm::ivec3> results;

	for (const glm::ivec3& p : node->prevPath)
	{
		results.push_back(p);
	}

	return results;
}

DungeonGeneratorPathfinder::PathCost DungeonGeneratorPathfinder::CostFunction(Node* n1, Node* n2, DungeonGrid<CellType>& dungeonGrid, const glm::ivec3 startPos, const glm::ivec3& endPos)
{
	PathCost pathCost;

	glm::ivec3 diff = n2->pos - n1->pos;
	pathCost.cost = glm::distance(glm::vec3(n2->pos), glm::vec3(endPos)); // Heuristic
	pathCost.traversable = true;
	return pathCost;

	//if (diff.y == 0) // No difference in y, this means that we are a flat hallway
	//{
	//	pathCost.cost = glm::distance(glm::vec3(n2->pos), glm::vec3(endPos)); // Heuristic

	//	if (dungeonGrid.Get(n2->pos) == CellType::Stairs)
	//	{
	//		return pathCost;
	//	}
	//	else if (dungeonGrid.Get(n2->pos) == CellType::Room)
	//	{
	//		pathCost.cost += 5; // Going through rooms are expensive
	//	}
	//	else if (dungeonGrid.Get(n2->pos) == CellType::None)
	//	{
	//		pathCost.cost += 1;
	//	}

	//	pathCost.traversable = true;
	//}
	//else // Y difference, probably stairs
	//{
	//	bool isNone = dungeonGrid.Get(n1->pos) == CellType::None;
	//	bool isHallway = dungeonGrid.Get(n1->pos) == CellType::Hallway;
	//	
	//	bool isNeighbourNone = dungeonGrid.Get(n2->pos) == CellType::None;
	//	bool isNeighbourHallway = dungeonGrid.Get(n2->pos) == CellType::Hallway;

	//	if ((!isNone && !isHallway) || (!isNeighbourNone && !isNeighbourHallway))
	//	{
	//		return pathCost;
	//	}

	//	pathCost.cost = 100.0f + glm::distance(glm::vec3(n2->pos), glm::vec3(endPos)); // Base cost + Heuristic

	//	int xDir = glm::clamp(diff.x, -1, 1);
	//	int zDir = glm::clamp(diff.z, -1, 1);

	//	glm::ivec3 verticalOffset(0, diff.y, 0);
	//	glm::ivec3 horizontalOffset(xDir, 0, zDir);

	//	if (!dungeonGrid.InBounds(n1->pos + verticalOffset)
	//		|| !dungeonGrid.InBounds(n1->pos + horizontalOffset)
	//		|| !dungeonGrid.InBounds(n1->pos + verticalOffset + horizontalOffset))
	//	{
	//		return pathCost;
	//	}

	//	if (dungeonGrid.Get(n1->pos + horizontalOffset) != CellType::None
	//		|| dungeonGrid.Get(n1->pos + horizontalOffset * 2) != CellType::None
	//		|| dungeonGrid.Get(n1->pos + verticalOffset + horizontalOffset) != CellType::None
	//		|| dungeonGrid.Get(n1->pos + verticalOffset + horizontalOffset * 2) != CellType::None)
	//	{
	//		return pathCost;
	//	}

	//	pathCost.traversable = true;
	//	pathCost.isStairs = true;
	//}

	//return pathCost;
}

DungeonGeneratorPathfinder::Node* DungeonGeneratorPathfinder::FindNextNode(std::priority_queue<Node*, std::vector<Node*>, NodeCompare>& queue, std::unordered_map<Node*, float>& map)
{
	Node* n = queue.top();
	std::unordered_map<Node*, float>::iterator queueIt = map.find(n);

	if (queueIt == map.end()) // Not in map, just return the node
	{
		queue.pop();
		return n;
	}

	float diff = n->cost - queueIt->second;
	if (diff > 0.001f) // Costs are different, pop and try agian
	{
		queue.pop();
		return FindNextNode(queue, map);
	}
	else // Same cost
	{
		queue.pop();
		return n;
	}
}