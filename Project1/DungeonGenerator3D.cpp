#include "DungeonGenerator3D.h"
#include "Utils.h"
#include "DungeonGenUtils.h"
#include "DungeonGeneratorPathfinder3D.h"
#include "Components.h"
#include "Mesh.h"

DungeonGenerator3D::DungeonGenerator3D(const DungeonGeneratorInfo& info, EntityManager& entityManager)
	: roomCount(info.roomCount),
	maxRoomSize(info.maxRoomSize),
	dungeonSize(info.dungeonSize),
	dungeonGrid(info.dungeonSize, glm::ivec3(0, 0, 0)),
	extraPathChance(info.extraPathChance),
	entityManager(entityManager),
	cubeMesh(new Mesh("assets/models/cube.obj")),
	wallMesh(new Mesh("assets/models/wall.obj")),
	floorMesh(new Mesh("assets/models/floor.obj"))
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

std::vector<Entity*> DungeonGenerator3D::Generate()
{
	std::vector<Entity*> entities;

 	GenerateRooms(entities);
	Tetrahedralize(entities);
	CreateHallways(entities);
	PathfindHallways(entities);

	float posScale = 10.0f;
	const float wallOffset = 0.5f * posScale;
	const glm::vec3 meshScale = glm::vec3(0.002f) * posScale;
	const glm::vec3 stairScale = glm::vec3(0.4f) * posScale;

	for (int x = 0; x < dungeonSize.x; x++)
	{
		float xPos = x * posScale;
		for (int y = 0; y < dungeonSize.y; y++)
		{
			float yPos = y * posScale;
			for (int z = 0; z < dungeonSize.z; z++)
			{
				float zPos = z * posScale;

				CellType type = dungeonGrid.Get(glm::ivec3(x, y, z));

				if (type == CellType::Hallway) // We are a hallway, we now need to get the cells next to us on the x and z axis
				{
					CellType leftCell = !dungeonGrid.InBounds(glm::ivec3(x - 1, y, z)) ? CellType::None : dungeonGrid.Get(glm::ivec3(x - 1, y, z));
					CellType rightCell = !dungeonGrid.InBounds(glm::ivec3(x + 1, y, z)) ? CellType::None : dungeonGrid.Get(glm::ivec3(x + 1, y, z));
					CellType backCell = !dungeonGrid.InBounds(glm::ivec3(x, y, z - 1)) ? CellType::None : dungeonGrid.Get(glm::ivec3(x, y, z + 1));
					CellType frontCell = !dungeonGrid.InBounds(glm::ivec3(x, y, z + 1)) ? CellType::None : dungeonGrid.Get(glm::ivec3(x, y, z - 1));

					/////////////////////////////////
					// BEGIN FLAT HALLWAY CHECKS
					/////////////////////////////////
					if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && backCell == CellType::Hallway && frontCell == CellType::Hallway) // Cross road
					{
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// TODO: Place "cross road" prest
					}
					else if ((leftCell == CellType::Hallway || rightCell == CellType::Hallway) && backCell == CellType::None && frontCell == CellType::None) // Straight hallway going down X
					{
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos + wallOffset), meshScale));
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/710ec568a95561bb22e51a6412a7fb8f
					}
					else if (leftCell == CellType::None && rightCell == CellType::None && (backCell == CellType::Hallway || frontCell == CellType::Hallway)) // Straight hallway going down 
					{
						entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
						entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/034c62a3becc8e642b2424b1d5985482
					}

					// L-shaped hallways
					else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::None)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos + wallOffset), meshScale));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/f7537b1bab7d4c830f3ca7d6bb256428
					}
					else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos + wallOffset), meshScale));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/deb9a057610f8dafab681936910cbfbd
					}
					else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Hallway)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/31c8d2a173358005c9f9d0cce71a5df6
					}
					else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/01a13d63c6f3abd756e4b021ad2c9747
					}

					// T-Shaped hallways
					else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Hallway)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/44c213c0dc267decf0332c7c82fa951b
					}
					else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::Hallway)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yPos, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/f4ca9244c4cbfccbc7deb845045aa1d2
					}
					else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos + wallOffset), meshScale));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/ee6ec031b1b8642848a8afaa110bbb52
					}
					else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
					{
						entities.push_back(PrepareWall(glm::vec3(xPos, yPos, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
						entities.push_back(PrepareFloor(glm::vec3(xPos, yPos, zPos), meshScale));
						// https://gyazo.com/01a13d63c6f3abd756e4b021ad2c9747
					}
					/////////////////////////////////
					// END FLAT HALLWAY CHECKS
					/////////////////////////////////

				}
				else if (type == CellType::Stairs)
				{
					entities.push_back(PrepareStairs(glm::vec3(xPos, yPos, zPos), stairScale));
				}
			}
		}
	}
	return entities;
}

void DungeonGenerator3D::GenerateRooms(std::vector<Entity*>& entities)
{
	for (int i = 0; i < roomCount; i++)
	{
		glm::ivec3 roomPos(Utils::RandInt(0, dungeonSize.x), Utils::RandInt(0, dungeonSize.y), Utils::RandInt(0, dungeonSize.z));
		glm::ivec3 roomSize(Utils::RandInt(1, maxRoomSize.x), Utils::RandInt(1, maxRoomSize.y), Utils::RandInt(1, maxRoomSize.z));

		bool validRoom = true;
		DungeonRoom newRoom(roomPos, roomSize);

		std::unordered_map<glm::ivec3, DungeonRoom>::iterator it;
		for (it = dungeonRooms.begin(); it != dungeonRooms.end(); it++)
		{
			DungeonRoom& room = it->second;
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

		dungeonGrid.Set(newRoom.center, CellType::Room);
		dungeonRooms.insert({roomPos, newRoom});
		entities.push_back(PrepareRoom(newRoom.center, newRoom.size));
	}
}

void DungeonGenerator3D::Tetrahedralize(std::vector<Entity*>& entities)
{
	std::vector<glm::vec3> vertices;
	vertices.resize(dungeonRooms.size());

	int i = 0;
	std::unordered_map<glm::ivec3, DungeonRoom>::iterator it;
	for (it = dungeonRooms.begin(); it != dungeonRooms.end(); it++)
	{
		DungeonRoom& room = it->second;
		vertices[i++] = room.center;
	}

	delaunay.vertices = vertices;
	delaunay.Tetrahedralize();
}

void DungeonGenerator3D::CreateHallways(std::vector<Entity*>& entities)
{
	std::vector<Edge> edges;
	edges.resize(delaunay.edges.size());

	for (unsigned int i = 0; i < edges.size(); i++)
	{
		edges[i] = delaunay.edges[i];
	}

	// Give use the essential hallways that are required to traverse to all rooms
	std::vector<Edge> minSpanningTree = DungeonGenUtils::MinimumSpanningTree(edges, edges[0].a);

	// Add random "non-essential" edges
	for (const Edge& e : edges)
	{
		if (std::find(mst.begin(), mst.end(), e) != mst.end()) continue; // This edge was already added

		float roll = Utils::RandFloat(0.0f, 1.0f);
		if (roll <= extraPathChance) mst.push_back(e); // Chance to add "non-essential" hallways
	}

	// Move the edges to the edge of the room
	for (unsigned int i = 0; i < minSpanningTree.size(); i++)
	{
		const Edge& e = minSpanningTree[i];
		glm::ivec3 posA(e.a);
		glm::ivec3 posB(e.b);

		if (dungeonGrid.Get(posA) == CellType::Room && dungeonGrid.Get(posB) == CellType::Room) // Found a room, move the edges to the edge of the room
		{
			DungeonRoom& roomA = dungeonRooms[posA];
			DungeonRoom& roomB = dungeonRooms[posB];

			Edge newEdge(posA, posB);

			// Room A
			glm::ivec3 dirA = roomB.center - roomA.center;
			if (glm::abs(dirA.x) > glm::abs(dirA.z)) // Edge starts at x axis
			{
				int x = dirA.x < 0 ? -roomA.size.x : roomA.size.x;
				newEdge.a = roomA.center + glm::ivec3(x, 0, 0);
			}
			else // Edge starts at z axis
			{
				int z = dirA.z < 0 ? -roomA.size.z : roomA.size.z;
				newEdge.a = roomA.center + glm::ivec3(0, 0, z);
			}

			// Room B
			glm::ivec3 dirB = roomA.center - roomB.center;
			if (glm::abs(dirB.x) > glm::abs(dirB.z)) // Edge starts at x axis
			{
				int x = dirB.x < 0 ? -roomB.size.x : roomB.size.x;
				newEdge.b = roomB.center + glm::ivec3(x, 0, 0);
			}
			else // Edge starts at z axis
			{
				int z = dirB.z < 0 ? -roomB.size.z : roomB.size.z;
				newEdge.b = roomB.center + glm::ivec3(0, 0, z);
			}

			mst.push_back(newEdge);
		}
		else // Not a room somehow, just assign the edge to the delaunay edge
		{
			mst.push_back(e);
		}
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

void DungeonGenerator3D::PathfindHallways(std::vector<Entity*>& entities)
{
	DungeonGeneratorPathfinder3D pathFinder(dungeonSize);

	// Pathfind between all edges in the minimum spanning tree
	int i = 0;
	for (Edge& e : mst)
	{
		glm::ivec3 start = e.a;
		glm::ivec3 end = e.b;

	//	if (i == 1) break;

		std::vector<glm::ivec3> path = pathFinder.Pathfind(start, end, dungeonGrid);
		std::cout << "Done path\n";
		if (!path.empty()) // We got a path!
		{
			for (unsigned int i = 0; i < path.size(); i++)
			{
				const glm::ivec3& current = path[i];

				if (dungeonGrid.Get(current) == CellType::None) // No cell here
				{
					dungeonGrid.Set(current, CellType::Hallway);
				}

				if (i > 0) // Check if we should be stairs
				{
					const glm::ivec3& prev = path[i - 1];
					glm::ivec3 diff = current - prev;

					if (diff.y != 0) // We are on different height level
					{
						dungeonGrid.Set(current, CellType::Stairs);
						//dungeonGrid.Set(glm::ivec3(current.x, current.y - 1, current.z), CellType::Stairs);
					}
				}
			}
		}

		i++;
	}
}

Entity* DungeonGenerator3D::PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size)
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

Entity* DungeonGenerator3D::PrepareHallway(const glm::ivec3& pos)
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

Entity* DungeonGenerator3D::PrepareStairs(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(glm::vec3(pos));
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>(rot);

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = cubeMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.0f, 0.4f, 0.0f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}

Entity* DungeonGenerator3D::PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos);
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>(rot);

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = wallMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.0f, 0.2f, 0.0f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}

Entity* DungeonGenerator3D::PrepareFloor(const glm::vec3& pos, const glm::vec3& scale)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos);
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>();

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = floorMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = glm::vec3(0.0f, 0.2f, 0.0f);
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}