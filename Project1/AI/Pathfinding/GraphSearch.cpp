#include "GraphSearch.h"

std::vector<glm::ivec2> GraphSearch::Dijkstra(Graph* graph, Graph::Node* startNode, Graph::NodeType searchType)
{
	graph->Reset();

	startNode->cost = 0;
	std::unordered_set<Graph::Node*> openSet;
	std::unordered_set<Graph::Node*> closedSet;
	openSet.insert(startNode);

	while (!openSet.empty())
	{
		Graph::Node* currentNode = GetLowestCostNode(openSet);
		openSet.erase(currentNode);
		closedSet.insert(currentNode);

		currentNode->visited = true;
		if (currentNode->type == searchType) // We found what we were looking for
		{
			return ReconstructPath(currentNode);
		}

		// Go through neighbouring nodes
		for (std::pair<Graph::Node*, float>& pair : currentNode->edges)
		{
			if (!pair.first->visited) // Visit the node
			{
				float weight = currentNode->cost + pair.second;
				if (weight < pair.first->cost)
				{
					pair.first->cost = weight;
					pair.first->prev = currentNode;
					if (openSet.find(pair.first) == openSet.end()) openSet.insert(pair.first);
				}
			}
		}
	}

	return std::vector<glm::ivec2>(); // Couldn't find path
}

std::vector<glm::ivec2> GraphSearch::AStar(Graph* graph, Graph::Node* startNode, Graph::Node* end)
{
	graph->Reset();
	startNode->cost = 0;
	startNode->hDistance = glm::distance(glm::vec2(startNode->pos), glm::vec2(end->pos));

	std::unordered_set<Graph::Node*> openSet;
	std::unordered_set<Graph::Node*> closedSet;
	openSet.insert(startNode);

	while (!openSet.empty())
	{
		Graph::Node* currentNode = GetLowestFCostNode(openSet);
		openSet.erase(currentNode);
		closedSet.insert(currentNode);

		currentNode->visited = true;
		if (currentNode == end) // We found what we were looking for
		{
			return ReconstructPath(currentNode);
		}

		// Go through neighbouring nodes
		for (std::pair<Graph::Node*, float>& pair : currentNode->edges)
		{
			if (!pair.first->visited) // Visit the node
			{
				float weight = currentNode->cost + pair.second;
				if (weight < pair.first->cost)
				{
					pair.first->cost = weight;
					pair.first->prev = currentNode;
					if (openSet.find(pair.first) == openSet.end())
					{
						pair.first->hDistance = glm::distance(glm::vec2(pair.first->pos), glm::vec2(end->pos));
						openSet.insert(pair.first);
					}
				}
			}
		}
	}

	return std::vector<glm::ivec2>(); // Couldn't find path
}

Graph::Node* GraphSearch::GetLowestCostNode(const std::unordered_set<Graph::Node*>& nodes)
{
	float minCost = std::numeric_limits<float>::max();
	Graph::Node* bestNode = nullptr;

	std::unordered_set<Graph::Node*>::const_iterator it = nodes.begin();
	while (it != nodes.end())
	{
		Graph::Node* n = *it;
		if (n->cost < minCost)
		{
			minCost = n->cost;
			bestNode = n;
		}

		it++;
	}

	return bestNode;
}

Graph::Node* GraphSearch::GetLowestFCostNode(const std::unordered_set<Graph::Node*>& nodes)
{
	float minCost = std::numeric_limits<float>::max();
	Graph::Node* bestNode = nullptr;

	std::unordered_set<Graph::Node*>::const_iterator it = nodes.begin();
	while (it != nodes.end())
	{
		Graph::Node* n = *it;
		if (n->cost + n->hDistance < minCost)
		{
			minCost = n->cost + n->hDistance;
			bestNode = n;
		}

		it++;
	}

	return bestNode;
}

std::vector<glm::ivec2> GraphSearch::ReconstructPath(Graph::Node* end)
{
	std::vector<glm::ivec2> path;
	Graph::Node* n = end;
	while (n)
	{
		path.push_back(n->pos);
		n = n->prev;
	}

	return path;
}