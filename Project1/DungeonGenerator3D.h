#pragma once

#include "AABB.h"
#include "Grid3D.h"
#include "Delaunay3D.h"
#include "EntityManager.h"

#include <glm/glm.hpp>

#include <vector>

class Mesh;
class DungeonGenerator3D
{
public:
	struct DungeonGeneratorInfo
	{
		int roomCount;
		glm::ivec3 maxRoomSize;
		glm::ivec3 dungeonSize;
		float extraPathChance;
	};

	DungeonGenerator3D(const DungeonGeneratorInfo& info, EntityManager& entityManager);

	std::vector<Entity*> Generate();

private:
	void GenerateRooms(std::vector<Entity*>& entities);
	void Tetrahedralize(std::vector<Entity*>& entities);
	void CreateHallways(std::vector<Entity*>& entities);
	void PathfindHallways(std::vector<Entity*>& entities);

	Entity* PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size);
	Entity* PrepareHallway(const glm::ivec3& pos);
	Entity* PrepareStairs(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
	Entity* PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
	Entity* PrepareFloor(const glm::vec3& pos, const glm::vec3& scale);

	int roomCount;
	const float extraPathChance;

	glm::ivec3 maxRoomSize;

	glm::ivec3 dungeonSize;

	std::unordered_map<glm::ivec3, DungeonRoom> dungeonRooms;
	std::vector<Edge> mst;

	Delaunay3D delaunay;

	Grid3D<CellType> dungeonGrid;

	EntityManager& entityManager;
	Mesh* cubeMesh;
	Mesh* wallMesh;
	Mesh* floorMesh;
};