#pragma once

#include "ApplicationLayer.h"
#include "Graph.h"
#include "EntityManager.h"
#include "Mesh.h"

#include <glm/gtx/hash.hpp>

#include <vector>
#include <unordered_map>

enum class AgentState
{
	Idle,
	Searching,
	Gathering,
	Returning,
	Storing
};

struct Agent
{
	AgentState state = AgentState::Idle;
	std::vector<glm::ivec2> path;
	int currentPathIndex = 0;
	float gatheringStartTime = 0.0;
	float storingStartTime = 0.0f;
	Entity* entity;
	Entity* resourceEntity = nullptr;
};

class PathfindingLayer : public ApplicationLayer
{
public:
	PathfindingLayer(const std::string& gridPath, Mesh* cubeMesh, Mesh* sphereMesh, EntityManager& entityManager);
	virtual ~PathfindingLayer();

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	void OnGather(Agent* agent, const glm::ivec2& pos);
	Graph::Node* GetStartNode(Agent& agent);
	Graph graph;
	EntityManager& entityManager;
	Mesh* cubeMesh;
	Mesh* sphereMesh;

	std::vector<Agent> agents;
	std::vector<Entity*> resources;
	std::vector<glm::ivec2> homes;
	std::unordered_map<glm::ivec2, Entity*> resourceMap;
};