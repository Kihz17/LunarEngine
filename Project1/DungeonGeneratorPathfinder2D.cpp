#include "DungeonGeneratorPathfinder2D.h"
#include "Utils.h"

constexpr glm::ivec2 neighbours[] = // Represents the directions we can move in on the grid
{
	glm::ivec2(1, 0),
	glm::ivec2(-1, 0),

	glm::ivec2(0, 1),
	glm::ivec2(0, -1),
};

DungeonGeneratorPathfinder2D::DungeonGeneratorPathfinder2D(const glm::ivec2& size)
	: grid(size, glm::ivec2(0, 0)),
	size(size)
{
	// Fill grid with nodes
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			grid.Set(glm::ivec2(x, y), new Node(glm::ivec2(x, y)));
		}
	}
}

DungeonGeneratorPathfinder2D::~DungeonGeneratorPathfinder2D()
{
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			delete grid.Get(glm::ivec2(x, y));
		}
	}
}

std::vector<glm::ivec2> DungeonGeneratorPathfinder2D::Pathfind(const glm::ivec2& start, const glm::ivec2& end, Grid2D<CellType>& dungeonGrid)
{
	ResetNodes();
	std::priority_queue<Node*, std::vector<Node*>, NodeCompare> queue;
	std::unordered_map<Node*, float> updatedQueueValues;
	std::unordered_set<Node*> closedSet;

	// Start us off at the start position
	grid.Get(start)->cost = 0.0f;
	queue.push(grid.Get(start));

	while (!queue.empty())
	{
		Node* fromNode = FindNextNode(queue, updatedQueueValues); // Find the next best node
		if (fromNode->pos == end)
		{
			return ReconstructPath(fromNode);
		}

		for (const glm::ivec2& n : neighbours)
		{
			glm::ivec2 currentPos = fromNode->pos + n;
			if (!grid.InBounds(currentPos)) continue; // This position isn't in our grid

			Node* currentNode = grid.Get(currentPos);
			if (closedSet.find(currentNode) != closedSet.end()) continue; // We have already traversed here, don't re-caculate path cost

			PathCost pathCost = CostFunction(fromNode, currentNode, dungeonGrid, start, end);
			if (!pathCost.traversable) continue; // Can't go here

			float newCost = fromNode->cost + pathCost.cost;
			if (newCost < currentNode->cost)
			{
				// Update the node's cost and put it in the queue
				currentNode->cost = pathCost.cost;
				queue.push(currentNode);
				updatedQueueValues.insert({ currentNode, pathCost.cost });

				currentNode->prev = fromNode;
			}
		}
	}

	return std::vector<glm::ivec2>(); // Couldn't form path
}

void DungeonGeneratorPathfinder2D::ResetNodes()
{
	glm::ivec2 gridSize = grid.Size();

	for (int x = 0; x < gridSize.x; x++)
	{
		for (int y = 0; y < gridSize.y; y++)
		{
			Node*& node = grid.Get(glm::ivec2(x, y));
			node->prev = nullptr;
			node->cost = std::numeric_limits<float>::max();
			node->randCost = Utils::RandFloat(0.0f, 10.0f);
		}
	}
}

std::vector<glm::ivec2> DungeonGeneratorPathfinder2D::ReconstructPath(Node* node)
{
	std::vector<glm::ivec2> results;

	Node* n = node;
	while (n != nullptr)
	{
		results.push_back(n->pos);
		n = n->prev;
	}

	return results;
}

DungeonGeneratorPathfinder2D::PathCost DungeonGeneratorPathfinder2D::CostFunction(Node* n1, Node* n2, Grid2D<CellType>& dungeonGrid, const glm::ivec2 startPos, const glm::ivec2& endPos)
{
	PathCost pathCost;
	pathCost.traversable = true;

	glm::ivec2 diff = n2->pos - n1->pos;

	pathCost.cost = glm::distance(glm::vec3(n2->pos, 0.0f), glm::vec3(endPos, 0.0f)) + n2->randCost; // Heuristic

	if (dungeonGrid.Get(n2->pos) == CellType::Room) // Going through rooms are expensive
	{
		pathCost.cost += 5;
	}
	else if (dungeonGrid.Get(n2->pos) == CellType::None)
	{
		pathCost.cost += 1;
	}

	return pathCost;
}

DungeonGeneratorPathfinder2D::Node* DungeonGeneratorPathfinder2D::FindNextNode(std::priority_queue<Node*, std::vector<Node*>, NodeCompare>& queue, std::unordered_map<Node*, float>& map)
{
	Node* n = queue.top();
	std::unordered_map<Node*, float>::iterator queueIt = map.find(n);

	if (queueIt == map.end()) // Not in map, just return the node
	{
		queue.pop();
		return n;
	}

	float diff = glm::abs(n->cost - queueIt->second);
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