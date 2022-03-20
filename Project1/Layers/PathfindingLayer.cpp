#include "PathfindingLayer.h"
#include "PositionComponent.h"
#include "ScaleComponent.h"
#include "RotationComponent.h"
#include "RenderComponent.h"
#include "GraphSearch.h"
#include "Utils.h"
#include "InputManager.h"

glm::vec3 GetNodeTypeColor(Graph::NodeType type)
{
	if (type == Graph::NodeType::Difficult)
	{
		return glm::vec3(1.0f, 1.0f, 0.0f);
	}
	else if (type == Graph::NodeType::Wall)
	{
		return glm::vec3(0.1f, 0.1f, 0.1f);
	}
	else if (type == Graph::NodeType::StartPosition)
	{
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else if (type == Graph::NodeType::Resource)
	{
		return glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (type == Graph::NodeType::Home)
	{
		return glm::vec3(0.0f, 0.0f, 1.0f);
	}
	else
	{
		return glm::vec3(1.0f, 1.0f, 1.0f);
	}
}

float agentSpeed = 2.0f;

PathfindingLayer::PathfindingLayer(const std::string& gridPath, Mesh* cubeMesh, Mesh* sphereMesh, EntityManager& entityManager)
	: graph(gridPath),
	entityManager(entityManager),
	cubeMesh(cubeMesh),
	sphereMesh(sphereMesh)
{

}

PathfindingLayer::~PathfindingLayer()
{

}

void PathfindingLayer::OnAttach()
{
	Grid2D<Graph::Node*>* grid = graph.GetGrid();
	for (int x = 0; x < grid->Size().x; x++)
	{
		for (int z = 0; z < grid->Size().y; z++)
		{
			Graph::Node* n = grid->Get(glm::ivec2(x, z));
			if (n->type == Graph::NodeType::StartPosition)
			{
				Entity* agentEntity = entityManager.CreateEntity();
				agentEntity->AddComponent<PositionComponent>(glm::vec3(x, 1.0f, z));
				agentEntity->AddComponent<RotationComponent>();
				agentEntity->AddComponent<ScaleComponent>(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::RenderInfo renderInfo;
				renderInfo.mesh = sphereMesh;
				renderInfo.isColorOverride = true;
				renderInfo.colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
				agentEntity->AddComponent<RenderComponent>(renderInfo);

				Agent agent;
				agent.entity = agentEntity;
				agents.push_back(agent);
			}
			else if (n->type == Graph::NodeType::Resource) 
			{
				Entity* resourceEntity = entityManager.CreateEntity();
				resourceEntity->AddComponent<PositionComponent>(glm::vec3(x, 1.0f, z));
				resourceEntity->AddComponent<RotationComponent>();
				resourceEntity->AddComponent<ScaleComponent>(glm::vec3(0.2f, 0.2f, 0.2f));

				RenderComponent::RenderInfo renderInfo;
				renderInfo.mesh = sphereMesh;
				renderInfo.isColorOverride = true;
				renderInfo.colorOverride = glm::vec3(0.2f, 0.8f, 0.8f);
				resourceEntity->AddComponent<RenderComponent>(renderInfo);

				resourceMap.insert({ glm::ivec2(x, z), resourceEntity });
			}
			else if (n->type == Graph::NodeType::Home)
			{
				homes.push_back(glm::ivec2(x, z));
			}

			Entity* e = entityManager.CreateEntity();
			e->AddComponent<PositionComponent>(glm::vec3(x, 0.0f, z));
			e->AddComponent<RotationComponent>();
			e->AddComponent<ScaleComponent>(glm::vec3(0.45f, 0.45f, 0.45f));

			RenderComponent::RenderInfo renderInfo;
			renderInfo.mesh = cubeMesh;
			renderInfo.isColorOverride = true;
			renderInfo.colorOverride = GetNodeTypeColor(n->type);

			e->AddComponent<RenderComponent>(renderInfo);
		}
	}
}

void PathfindingLayer::OnUpdate(float deltaTime)
{
	for (Agent& agent : agents)
	{
		glm::vec3 currentPos = agent.entity->GetComponent<PositionComponent>()->value;

		if (agent.state == AgentState::Idle && !resourceMap.empty())
		{
			Graph::Node* startNode = GetStartNode(agent);
			if (!startNode) continue;

			agent.path = GraphSearch::Dijkstra(&graph, startNode, Graph::NodeType::Resource);
			if (!agent.path.empty())
			{
				agent.currentPathIndex = agent.path.size() - 1;
				agent.state = AgentState::Searching;
			}
		}
		else if (agent.state == AgentState::Gathering)
		{
			if (glfwGetTime() - agent.gatheringStartTime >= 4.0f)
			{
				if(!agent.path.empty()) OnGather(&agent, agent.path[0]);
				Graph::Node* destNode = graph.GetGrid()->Get(homes[Utils::RandInt(0, homes.size() - 1)]);

				// Path back to home
				agent.path = GraphSearch::AStar(&graph, graph.GetGrid()->Get(glm::ivec2(currentPos.x, currentPos.z)), destNode);
				if (!agent.path.empty())
				{
					agent.currentPathIndex = agent.path.size() - 1;
					agent.state = AgentState::Returning;
				}
			}
		}
		else if (agent.state == AgentState::Storing)
		{
			if (glfwGetTime() - agent.storingStartTime >= 2.0f)
			{
				// Remove resource
				if (agent.resourceEntity)
				{
					entityManager.DeleteEntity(agent.resourceEntity);
					agent.resourceEntity = nullptr;
					agent.state = AgentState::Idle; // Try to look for another resource
				}
			}
		}
		else if (agent.state == AgentState::Searching)
		{
			glm::ivec2 destination = agent.path[agent.currentPathIndex];

			if (glm::distance2(currentPos, glm::vec3(destination.x, currentPos.y, destination.y)) < 1.0f) // Arrived, move to next
			{
				agent.currentPathIndex--;
				if (agent.currentPathIndex < 0) // End of path, start gathering
				{
					agent.state = AgentState::Gathering;
					agent.gatheringStartTime = glfwGetTime();
					continue;
				}

				destination = agent.path[agent.currentPathIndex];
			}

			// Move towards destination
			glm::vec3 velocity = glm::normalize(glm::vec3(destination.x, 1.0f, destination.y) - currentPos) * agentSpeed * deltaTime;
			agent.entity->GetComponent<PositionComponent>()->value += velocity;
		}
		else if (agent.state == AgentState::Returning)
		{
			glm::ivec2 destination = agent.path[agent.currentPathIndex];

			if (glm::distance2(currentPos, glm::vec3(destination.x, currentPos.y, destination.y)) < 1.0f) // Arrived, move to next
			{
				agent.currentPathIndex--;
				if (agent.currentPathIndex < 0) // End of path, start storing
				{
					agent.state = AgentState::Storing; // Switch to storing the resource
					agent.storingStartTime = glfwGetTime();
					continue;
				}

				destination = agent.path[agent.currentPathIndex];
			}

			// Move towards destination
			glm::vec3 velocity = glm::normalize(glm::vec3(destination.x, 1.0f, destination.y) - currentPos) * agentSpeed * deltaTime;
			agent.entity->GetComponent<PositionComponent>()->value += velocity;

			// Make resource follow agent
			if (agent.resourceEntity)
			{
				agent.resourceEntity->GetComponent<PositionComponent>()->value = agent.entity->GetComponent<PositionComponent>()->value + glm::vec3(0.0f, 1.0f, 0.0f);
			}
		}
	}

	if (InputManager::GetKey(GLFW_KEY_EQUAL)->IsPressed())
	{
		agentSpeed = glm::min(500.0f, agentSpeed + 0.1f);
	}
	else if (InputManager::GetKey(GLFW_KEY_MINUS)->IsPressed())
	{
		agentSpeed = glm::max(2.0f, agentSpeed - 0.1f);
	}
}

void PathfindingLayer::OnGather(Agent* agent, const glm::ivec2& pos)
{
	graph.GetGrid()->Get(pos)->type = Graph::NodeType::None;
	if (resourceMap.find(pos) == resourceMap.end()) return;

	Entity* resourceEntity = resourceMap.at(pos);
	resourceMap.erase(pos);

	agent->resourceEntity = resourceEntity; // Give resouece to agent

	for (Agent& a : agents)
	{
		if (a.path.empty() || a.path[0] != pos || &a == agent) continue;

		// Pathing to same resource, we need to change it
		a.state = AgentState::Idle; // Choose new path next frame
	}
}

Graph::Node* PathfindingLayer::GetStartNode(Agent& agent)
{
	glm::ivec2 size = graph.GetGrid()->Size();
	float closestDist = std::numeric_limits<float>::max();
	Graph::Node* closest = nullptr;

	glm::vec3 agentPos = agent.entity->GetComponent<PositionComponent>()->value;
	glm::vec2 agentP(agentPos.x, agentPos.z);

	for (int x = 0; x < size.x; x++)
	{
		for (int z = 0; z < size.y; z++)
		{
			glm::ivec2 pos(x, z);
			Graph::Node* n = graph.GetGrid()->Get(pos);
			if (n->type == Graph::NodeType::Wall) continue;

			float dist = glm::distance(glm::vec2(pos), agentP);
			if (dist < closestDist)
			{
				closestDist = dist;
				closest = n;
			}
		}
	}

	return closest;
}