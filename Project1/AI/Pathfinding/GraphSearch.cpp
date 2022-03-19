#include "GraphSearch.h"

Graph::Node* GraphSearch::Dijkstra(Graph* graph, Graph::Node* startNode, Graph::NodeType searchType)
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
			return currentNode;
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

	return nullptr; // Couldn't find path
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
	}

	return bestNode;
}