#pragma once

#include "DungeonGeneratorTypes.h"
#include "Grid2D.h"
#include "Delaunay2D.h"
#include "EntityManager.h"
#include "Mesh.h"
#include "RenderComponent.h"

#include <glm/glm.hpp>
#include <unordered_map>

class DungeonGenerator2D
{
public:
	struct DungeonGeneratorInfo
	{
		int roomCount;
		glm::ivec3 minRoomSize;
		glm::ivec3 maxRoomSize;
		glm::ivec2 dungeonSize;
		float extraPathChance;
		int yLevel;
		glm::ivec2 dungeonOffset = glm::ivec2(0, 0);
		float posScale;
		float wallOffset;
		glm::vec3 meshScale;
		RenderComponent::RenderInfo wallInfo;
		RenderComponent::RenderInfo floorInfo;
		RenderComponent::RenderInfo ceilingInfo;
		glm::quat floorRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float wallYOffset = 0.0f;
	};

	DungeonGenerator2D(const DungeonGeneratorInfo& info, EntityManager& entityManager);

	void Generate();
	std::vector<Entity*> PlaceEntities(const glm::vec2& startPos, const glm::ivec2& direction);

	Grid2D<CellType>& GetDungeonGrid() { return dungeonGrid; }

private:
	void GenerateRooms();
	void Triangulate();
	void CreateHallways();
	void PathfindHallways();

	Entity* PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), const glm::vec3& color = glm::vec3(0.0f, 0.2f, 0.0f));
	Entity* PrepareFloor(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& color = glm::vec3(0.0f, 0.4f, 0.0f));

	int roomCount;
	const float extraPathChance;
	glm::ivec3 minRoomSize;
	glm::ivec2 maxRoomSize;
	glm::ivec2 dungeonSize;
	int yLevel;
	glm::ivec2 positionOffset;

	Grid2D<CellType> dungeonGrid;
	Delaunay2D delaunay;
	std::unordered_map<glm::ivec2, DungeonRoom2D> dungeonRooms;
	std::vector<Edge2D> mst;

	float posScale;
	float wallOffset;
	glm::vec3 meshScale;
	glm::quat floorRot;
	float wallYOffset;

	EntityManager& entityManager;

	RenderComponent::RenderInfo wallInfo;
	RenderComponent::RenderInfo floorInfo;
	RenderComponent::RenderInfo ceilingInfo;
};