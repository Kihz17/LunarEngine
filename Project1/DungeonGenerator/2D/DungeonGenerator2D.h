#pragma once

#include "DungeonGeneratorTypes.h"
#include "Grid2D.h"
#include "Delaunay2D.h"
#include "EntityManager.h"
#include "Mesh.h"
 
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
	};

	DungeonGenerator2D(const DungeonGeneratorInfo& info, EntityManager& entityManager);

	std::vector<Entity*> Generate();

private:
	void GenerateRooms(std::vector<Entity*>& entities);
	void Triangulate(std::vector<Entity*>& entities);
	void CreateHallways(std::vector<Entity*>& entities);
	void PathfindHallways(std::vector<Entity*>& entities);

	Entity* PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size);
	Entity* PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), const glm::vec3& color = glm::vec3(0.0f, 0.2f, 0.0f));
	Entity* PrepareFloor(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& color = glm::vec3(0.0f, 0.4f, 0.0f));

	int roomCount;
	const float extraPathChance;
	glm::ivec3 minRoomSize;
	glm::ivec2 maxRoomSize;
	glm::ivec2 dungeonSize;
	int yLevel;

	Grid2D<CellType> dungeonGrid;
	Delaunay2D delaunay;
	std::unordered_map<glm::ivec2, DungeonRoom2D> dungeonRooms;
	std::vector<Edge2D> mst;

	EntityManager& entityManager;
	Mesh* cubeMesh;
	Mesh* wallMesh;
	Mesh* floorMesh;
};