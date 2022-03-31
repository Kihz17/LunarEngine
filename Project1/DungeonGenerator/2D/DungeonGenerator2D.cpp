#include "DungeonGenerator2D.h"
#include "Utils.h"
#include "DungeonGenUtils.h"
#include "Components.h"
#include "DungeonGeneratorPathfinder2D.h"
#include "MeshManager.h"

DungeonGenerator2D::DungeonGenerator2D(const DungeonGeneratorInfo& info, EntityManager& entityManager)
	: roomCount(info.roomCount),
	maxRoomSize(info.maxRoomSize),
	minRoomSize(info.minRoomSize),
	dungeonSize(info.dungeonSize),
	dungeonGrid(info.dungeonSize, glm::ivec2(0, 0)),
	extraPathChance(info.extraPathChance),
	entityManager(entityManager),
	yLevel(info.yLevel),
	positionOffset(info.dungeonOffset),
	posScale(info.posScale),
	wallOffset(info.wallOffset),
	wallYOffset(info.wallYOffset),
	floorRot(info.floorRot),
	meshScale(info.meshScale),
	wallInfo(info.wallInfo),
	floorInfo(info.floorInfo),
	ceilingInfo(info.ceilingInfo)
{
	for (int x = 0; x < dungeonSize.x; x++)
	{
		for (int z = 0; z < dungeonSize.y; z++)
		{
			dungeonGrid.Set(glm::ivec2(x, z), CellType::None);
		}
	}
}

void DungeonGenerator2D::Generate()
{
	GenerateRooms();
	Triangulate();
	CreateHallways();
	PathfindHallways();
}

std::vector<Entity*> DungeonGenerator2D::PlaceEntities(const glm::vec2& startPos, const glm::ivec2& direction)
{
	std::vector<Entity*> entities;

	// Convert start position to grid
	int gridX = (int)((startPos.x - positionOffset.x) / posScale);
	int gridZ = (int)((startPos.y - positionOffset.y) / posScale);
	glm::ivec2 gridPos(gridX, gridZ);

	glm::ivec2 entrance(-1, -1);
	while (true)
	{
		if (dungeonGrid.InBounds(gridPos) && dungeonGrid.Get(gridPos) != CellType::None)
		{
			entrance = gridPos;
			break;
		}

		// Add Entities
		float xPos = gridPos.x * posScale + positionOffset.x;
		float zPos = gridPos.y * posScale + positionOffset.y;
		entities.push_back(PrepareFloor(glm::vec3(xPos, yLevel, zPos), meshScale));

		if (direction.x != 0 && direction.y == 0)
		{
			entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
			entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
		}
		else if (direction.x == 0 && direction.y != 0)
		{
			entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
			entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
		}

		gridPos += direction;
	}

	for (int x = 0; x < dungeonSize.x; x++)
	{
		float xPos = x * posScale + positionOffset.x;
		for (int z = 0; z < dungeonSize.y; z++)
		{
			float zPos = z * posScale + positionOffset.y;

			CellType type = dungeonGrid.Get(glm::ivec2(x, z));

			CellType leftCell = !dungeonGrid.InBounds(glm::ivec2(x - 1, z)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x - 1, z));
			CellType rightCell = !dungeonGrid.InBounds(glm::ivec2(x + 1, z)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x + 1, z));
			CellType backCell = !dungeonGrid.InBounds(glm::ivec2(x, z - 1)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x, z + 1));
			CellType frontCell = !dungeonGrid.InBounds(glm::ivec2(x, z + 1)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x, z - 1));

			if (type == CellType::Hallway || type == CellType::Room)
			{
				entities.push_back(PrepareFloor(glm::vec3(xPos, yLevel, zPos), meshScale, glm::vec3(0.4f, 0.0f, 0.0f)));

				bool isLeftCellEntrance = (x == entrance.x && z == entrance.y) && direction == glm::ivec2(1, 0);
				if (leftCell == CellType::None && !isLeftCellEntrance)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}

				bool isRightCellEntrance = (x == entrance.x && z == entrance.y) && direction == glm::ivec2(-1, 0);
				if (rightCell == CellType::None && !isRightCellEntrance)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}

				bool isBackCellEntrance = (x == entrance.x && z == entrance.y) && direction == glm::ivec2(0, -1);
				if (backCell == CellType::None && !isBackCellEntrance)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}

				bool isFrontCellEntrance = (x == entrance.x && z == entrance.y) && direction == glm::ivec2(0, 1);
				if (frontCell == CellType::None && !isFrontCellEntrance)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
			}
		}
	}

	return entities;
}

void DungeonGenerator2D::GenerateRooms()
{
	for (int i = 0; i < roomCount; i++)
	{
		glm::ivec2 roomPos(Utils::RandInt(0, dungeonSize.x), Utils::RandInt(0, dungeonSize.y));
		glm::ivec2 roomSize(Utils::RandInt(minRoomSize.x, maxRoomSize.x), Utils::RandInt(minRoomSize.z, maxRoomSize.y));

		bool validRoom = true;
		DungeonRoom2D newRoom(roomPos, roomSize);

		std::unordered_map<glm::ivec2, DungeonRoom2D>::iterator it;
		for (it = dungeonRooms.begin(); it != dungeonRooms.end(); it++)
		{
			DungeonRoom2D& room = it->second;
			if (room.Intersects(newRoom)) // This rooms collides with another, skip it
			{
				validRoom = false;
				break;
			}
		}

		glm::ivec2 min = roomPos - roomSize;
		glm::ivec2 max = roomPos + roomSize;

		// Make sure the max and min points are within the arena
		if (min.x < 0 || max.x > dungeonSize.x || min.y < 0 || max.y > dungeonSize.y)
		{
			validRoom = false;
		}

		if (!validRoom) // Room not valid, retry
		{
			i--;
			continue;
		}

		for (int x = min.x; x < max.x; x++)
		{
			for (int z = min.y; z < max.y; z++)
			{
				dungeonGrid.Set(glm::ivec2(x, z), CellType::Room);
			}
		}

		dungeonRooms.insert({ roomPos, newRoom });
	}
}

void DungeonGenerator2D::Triangulate()
{
	std::vector<glm::vec2> vertices;
	vertices.resize(dungeonRooms.size());

	int i = 0;
	std::unordered_map<glm::ivec2, DungeonRoom2D>::iterator it;
	for (it = dungeonRooms.begin(); it != dungeonRooms.end(); it++)
	{
		DungeonRoom2D& room = it->second;
		vertices[i++] = room.pos;
	}

	delaunay.vertices = vertices;
	delaunay.Triangulate();
}

void DungeonGenerator2D::CreateHallways()
{
	std::vector<Edge2D> edges;
	edges.resize(delaunay.edges.size());

	for (unsigned int i = 0; i < edges.size(); i++)
	{
		edges[i] = delaunay.edges[i];
	}

	// Give use the essential hallways that are required to traverse to all rooms
	std::vector<Edge2D> minSpanningTree = DungeonGenUtils::MinimumSpanningTree2D(edges, edges[0].a);

	// Add random "non-essential" edges
	for (const Edge2D& e : edges)
	{
		if (std::find(mst.begin(), mst.end(), e) != mst.end()) continue; // This edge was already added

		float roll = Utils::RandFloat(0.0f, 1.0f);
		if (roll <= extraPathChance) mst.push_back(e); // Chance to add "non-essential" hallways
	}

	// Move the edges to the edge of the room
	for (unsigned int i = 0; i < minSpanningTree.size(); i++)
	{
		const Edge2D& e = minSpanningTree[i];

		if (dungeonGrid.Get(e.a) == CellType::Room && dungeonGrid.Get(e.b) == CellType::Room) // Found a room, move the edges to the edge of the room
		{
			DungeonRoom2D& roomA = dungeonRooms[e.a];
			DungeonRoom2D& roomB = dungeonRooms[e.b];

			Edge2D newEdge(e.a, e.b);

			// Room A
			glm::ivec2 dirA = roomB.pos - roomA.pos;
			if (glm::abs(dirA.x) > glm::abs(dirA.y)) // Edge starts at x axis
			{
				int x = dirA.x < 0 ? -roomA.size.x : roomA.size.x;
				newEdge.a = roomA.pos + glm::ivec2(x, 0);
			}
			else // Edge starts at z axis
			{
				int z = dirA.y < 0 ? -roomA.size.y : roomA.size.y;
				newEdge.a = roomA.pos + glm::ivec2(0, z);
			}

			// Room B
			glm::ivec2 dirB = roomA.pos - roomB.pos;
			if (glm::abs(dirB.x) > glm::abs(dirB.y)) // Edge starts at x axis
			{
				int x = dirB.x < 0 ? -roomB.size.x : roomB.size.x;
				newEdge.b = roomB.pos + glm::ivec2(x, 0);
			}
			else // Edge starts at z axis
			{
				int z = dirB.y < 0 ? -roomB.size.y : roomB.size.y;
				newEdge.b = roomB.pos + glm::ivec2(0, z);
			}

			mst.push_back(newEdge);
		}
		else // Not a room somehow, just assign the edge to the delaunay edge
		{
			mst.push_back(e);
		}
	}

	/*for (Edge2D& edge : mst)
	{
		Entity* e = entityManager.PrepareEntity();
		e->AddComponent<PositionComponent>();
		e->AddComponent<ScaleComponent>();
		e->AddComponent<RotationComponent>();
		e->AddComponent<LineRenderComponent>(glm::vec3(edge.a.x, yLevel, edge.a.y), glm::vec3(edge.b.x, yLevel, edge.b.y));
		entities.push_back(e);
	}*/
}

void DungeonGenerator2D::PathfindHallways()
{
	DungeonGeneratorPathfinder2D pathFinder(dungeonSize);

	for (Edge2D& e : mst)
	{
		glm::ivec2 start = e.a;
		glm::ivec2 end = e.b;

		std::vector<glm::ivec2> path = pathFinder.Pathfind(start, end, dungeonGrid);
		if (!path.empty()) // We got a path!
		{
			for (unsigned int i = 0; i < path.size(); i++)
			{
				const glm::ivec2& current = path[i];

				if (dungeonGrid.Get(current) != CellType::Room) // No cell here
				{
					dungeonGrid.Set(current, CellType::Hallway);
				}
			}
		}
	}
}

Entity* DungeonGenerator2D::PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot, const glm::vec3& color)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos + glm::vec3(0.0f, wallYOffset, 0.0f));
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>(rot);

	e->AddComponent<RenderComponent>(wallInfo);
	return e;
}

Entity* DungeonGenerator2D::PrepareFloor(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& color)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos);
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>(floorRot);

	e->AddComponent<RenderComponent>(floorInfo);
	return e;
}