#include "DungeonGenerator.h"
#include "Utils.h"
#include "DungeonGenUtils.h"
#include "DungeonGeneratorPathfinder.h"
#include "Components.h"
#include "Mesh.h"

DungeonGenerator::DungeonGenerator(int roomCount, const glm::ivec3& maxRoomSize, const glm::ivec3& dungeonSize, EntityManager& entityManager)
	: roomCount(roomCount),
	maxRoomSize(maxRoomSize),
	dungeonSize(dungeonSize),
	dungeonGrid(dungeonSize, glm::ivec3(0, 0, 0)),
	entityManager(entityManager),
	cubeMesh(new Mesh("assets/models/cube.obj"))
{
	for (int x = 0; x < dungeonSize.x; x++)
	{
		for (int y = 0; y < dungeonSize.y; y++)
		{
			for (int z = 0; z < dungeonSize.z; z++)
			{
				dungeonGrid.Set(glm::ivec3(x, y, z), CellType::None);
			}
		}
	}
}

std::vector<Entity*> DungeonGenerator::Generate()
{
	std::vector<Entity*> entities;

 	GenerateRooms(entities);
	Tetrahedralize(entities);
	CreateHallways(entities);
	PathfindHallways(entities);

	return entities;
}

void DungeonGenerator::GenerateRooms(std::vector<Entity*>& entities)
{
	for (int i = 0; i < roomCount; i++)
	{
		glm::ivec3 roomPos(Utils::RandInt(0, dungeonSize.x), Utils::RandInt(0, dungeonSize.y), Utils::RandInt(0, dungeonSize.z));
		glm::ivec3 roomSize(Utils::RandInt(1, maxRoomSize.x), Utils::RandInt(1, maxRoomSize.y), Utils::RandInt(1, maxRoomSize.z));

		bool validRoom = true;
		DungeonRoom newRoom(roomPos, roomSize);

		for (DungeonRoom& room : dungeonRooms)
		{
			if (room.Intersects(newRoom)) // This rooms collides with another, skip it
			{
				validRoom = false;
				break;
			}
		}

		glm::ivec3 min = newRoom.center - newRoom.size;
		glm::ivec3 max = newRoom.center + newRoom.size;

		// Make sure the max and min points are within the arena
		if (min.x < 0 || max.x > dungeonSize.x
			|| min.y < 0 || max.y > dungeonSize.y
			|| min.z < 0 || max.z > dungeonSize.z)
		{
			validRoom = false;
		}

		if (!validRoom) // Room not valid, retry
		{
			i--;
			continue;
		}

		dungeonRooms.push_back(newRoom);
		entities.push_back(PrepareRoom(newRoom.center, newRoom.size));
	}
}

void DungeonGenerator::Tetrahedralize(std::vector<Entity*>& entities)
{
	std::vector<glm::vec3> vertices;
	vertices.resize(dungeonRooms.size());
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		DungeonRoom& room = dungeonRooms[i];
		vertices[i] = room.center;
	}

	delaunay.vertices = vertices;
	delaunay.Tetrahedralize();
}

void DungeonGenerator::CreateHallways(std::vector<Entity*>& entities)
{
	std::vector<Edge> edges;
	edges.resize(delaunay.edges.size());

	for (unsigned int i = 0; i < edges.size(); i++)
	{
		edges[i] = delaunay.edges[i];
	}

	// Give use the essential hallways that are required to traverse to all rooms
	std::vector<Edge> minSpanningTree = DungeonGenUtils::MinimumSpanningTree(edges, edges[0].a);
	mst = minSpanningTree;

	for (const Edge& e : edges)
	{
		if (std::find(mst.begin(), mst.end(), e) != mst.end()) continue; // This edge was already added

		if (Utils::RandFloat(0.0f, 1.0f) < 0.125f) mst.push_back(e); // 12.5% chance to add "non-essential" hallways
	}

	for (Edge& edge : mst)
	{
		Entity* e = entityManager.PrepareEntity();
		e->AddComponent<PositionComponent>();
		e->AddComponent<ScaleComponent>();
		e->AddComponent<RotationComponent>();
		e->AddComponent<LineRenderComponent>(glm::vec3(edge.a), glm::vec3(edge.b));
		entities.push_back(e);
	}
}

void DungeonGenerator::PathfindHallways(std::vector<Entity*>& entities)
{
	DungeonGeneratorPathfinder pathFinder(dungeonSize);

	// Pathfind between all edges in the minimum spanning tree
	for (Edge& e : mst)
	{
		glm::ivec3 start = e.a;
		glm::ivec3 end = e.b;

		std::vector<glm::ivec3> path = pathFinder.Pathfind(start, end, dungeonGrid);
		if (!path.empty()) // We got a path!
		{
			std::cout << "Finished path!\n";
			for (unsigned int i = 0; i < path.size(); i++)
			{
				const glm::ivec3& current = path[i];

				if (dungeonGrid.Get(current) == CellType::None) // No cell here
				{
					dungeonGrid.Set(current, CellType::Hallway);
					entities.push_back(PrepareHallway(current));
				}

				//if (i > 0) // Check if we should be stairs
				//{
				//	const glm::ivec3& prev = path[i - 1];
				//	glm::ivec3 diff = current - prev;

				//	if (diff.y != 0) // We are on different height level
				//	{
				//		int xDir = glm::clamp(diff.x, -1, 1);
				//		int zDir = glm::clamp(diff.z, -1, 1);

				//		glm::ivec3 verticalOffset(0, diff.y, 0);
				//		glm::ivec3 horizontalOffset(xDir, 0, zDir);

				//		dungeonGrid.Set(prev + horizontalOffset, CellType::Stairs);
				//		dungeonGrid.Set(prev + horizontalOffset * 2, CellType::Stairs);
				//		dungeonGrid.Set(prev + verticalOffset + horizontalOffset, CellType::Stairs);
				//		dungeonGrid.Set(prev + verticalOffset + horizontalOffset * 2, CellType::Stairs);

				//		entities.push_back(PrepareStairs(prev + horizontalOffset));
				//		entities.push_back(PrepareStairs(prev + horizontalOffset * 2));
				//		entities.push_back(PrepareStairs(prev + verticalOffset + horizontalOffset));
				//		entities.push_back(PrepareStairs(prev + verticalOffset + horizontalOffset * 2));
				//	}
				//}
			}

		/*	for (const glm::ivec3& p : path)
			{
				if (!dungeonGrid.InBounds(p))
				{
					std::cout << "HY not in bounds!\n";
					continue;
				}
				if (dungeonGrid.Get(p) == CellType::Hallway)
				{
					entities.push_back(PrepareHallway(p));
				}
			}*/
		}
	}
}

Entity* DungeonGenerator::PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(glm::vec3(pos));
	e->AddComponent<ScaleComponent>(glm::vec3(size));
	e->AddComponent<RotationComponent>();

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = cubeMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.4f, 0.0f, 0.0f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}

Entity* DungeonGenerator::PrepareHallway(const glm::ivec3& pos)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(glm::vec3(pos));
	e->AddComponent<ScaleComponent>(glm::vec3(0.5f, 0.5f, 0.5f));
	e->AddComponent<RotationComponent>();

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = cubeMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.4f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}

Entity* DungeonGenerator::PrepareStairs(const glm::ivec3& pos)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(glm::vec3(pos));
	e->AddComponent<ScaleComponent>();
	e->AddComponent<RotationComponent>();

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = cubeMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.0f, 0.4f, 0.0f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}