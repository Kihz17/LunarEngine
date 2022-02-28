#pragma once

#include "AABB.h"
#include "Delaunay.h"
#include "DungeonGrid.h"
#include "EntityManager.h"

#include <glm/glm.hpp>

#include <vector>

struct DungeonRoom
{
	DungeonRoom(const glm::ivec3& center, const glm::ivec3& size)
		: center(center), size(size)
	{}

	bool Intersects(const DungeonRoom& otherRoom)
	{
		glm::ivec3 min = center - size;
		glm::ivec3 max = center + size;

		glm::ivec3 otherMin = otherRoom.center - otherRoom.size;
		glm::ivec3 otherMax = otherRoom.center + otherRoom.size;

		return (min.x <= otherMax.x && max.x >= otherMin.x)
			&& (min.y <= otherMax.y && max.y >= otherMin.y)
			&& (min.z <= otherMax.z && max.z >= otherMin.z);
	}

	glm::ivec3 center;
	glm::ivec3 size;
};

class Mesh;
class DungeonGenerator
{
public:
	DungeonGenerator(int roomCount, const glm::ivec3& maxRoomSize, const glm::ivec3& dungeonSize, EntityManager& entityManager);

	std::vector<Entity*> Generate();

private:
	void GenerateRooms(std::vector<Entity*>& entities);
	void Tetrahedralize(std::vector<Entity*>& entities);
	void CreateHallways(std::vector<Entity*>& entities);
	void PathfindHallways(std::vector<Entity*>& entities);

	Entity* PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size);
	Entity* PrepareHallway(const glm::ivec3& pos);
	Entity* PrepareStairs(const glm::ivec3& pos);

	int roomCount;

	glm::ivec3 maxRoomSize;

	glm::ivec3 dungeonSize;

	std::vector<DungeonRoom> dungeonRooms;
	std::vector<Edge> mst;

	Delaunay delaunay;

	DungeonGrid<CellType> dungeonGrid;

	EntityManager& entityManager;
	Mesh* cubeMesh;
};