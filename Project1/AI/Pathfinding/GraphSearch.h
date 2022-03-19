#pragma once

#include "Graph.h"

#include <unordered_set>

class GraphSearch
{
public:
	static Graph::Node* Dijkstra(Graph* graph, Graph::Node* startNode, Graph::NodeType searchType);
	static Graph::Node* GetLowestCostNode(const std::unordered_set<Graph::Node*>& node);
};