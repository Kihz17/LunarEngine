#pragma once

#include "Graph.h"

#include <unordered_set>

class GraphSearch
{
public:
	static std::vector<glm::ivec2> Dijkstra(Graph* graph, Graph::Node* startNode, Graph::NodeType searchType);
	static std::vector<glm::ivec2> AStar(Graph* graph, Graph::Node* startNode, Graph::Node* end);
	static Graph::Node* GetLowestCostNode(const std::unordered_set<Graph::Node*>& node);
	static Graph::Node* GetLowestFCostNode(const std::unordered_set<Graph::Node*>& node);
	static std::vector<glm::ivec2> ReconstructPath(Graph::Node* startNode);
};