#include "DungeonGenerator2D.h"
#include "Utils.h"
#include "DungeonGenUtils.h"
#include "Components.h"
#include "DungeonGeneratorPathfinder2D.h"

DungeonGenerator2D::DungeonGenerator2D(const DungeonGeneratorInfo& info, EntityManager& entityManager)
	: roomCount(info.roomCount),
	maxRoomSize(info.maxRoomSize),
	minRoomSize(info.minRoomSize),
	dungeonSize(info.dungeonSize),
	dungeonGrid(info.dungeonSize, glm::ivec2(0, 0)),
	extraPathChance(info.extraPathChance),
	entityManager(entityManager),
	yLevel(info.yLevel),
	cubeMesh(new Mesh("assets/models/cube.obj")),
	wallMesh(new Mesh("assets/models/wall.obj")),
	floorMesh(new Mesh("assets/models/floor.obj"))
{
	for (int x = 0; x < dungeonSize.x; x++)
	{
		for (int z = 0; z < dungeonSize.y; z++)
		{
			dungeonGrid.Set(glm::ivec2(x, z), CellType::None);
		}
	}
}

std::vector<Entity*> DungeonGenerator2D::Generate()
{
	std::vector<Entity*> entities;

	GenerateRooms(entities);
	Triangulate(entities);
	CreateHallways(entities);
	PathfindHallways(entities);
	 
	float posScale = 10.0f;
	const float wallOffset = 0.525f * posScale;
	const glm::vec3 meshScale = glm::vec3(0.002f) * posScale;
	const glm::vec3 stairScale = glm::vec3(0.4f) * posScale;

	for (int x = 0; x < dungeonSize.x; x++)
	{
		float xPos = x * posScale;
		for (int z = 0; z < dungeonSize.y; z++)
		{
			float zPos = z * posScale;

			CellType type = dungeonGrid.Get(glm::ivec2(x, z));

			CellType leftCell = !dungeonGrid.InBounds(glm::ivec2(x - 1, z)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x - 1, z));
			CellType rightCell = !dungeonGrid.InBounds(glm::ivec2(x + 1, z)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x + 1, z));
			CellType backCell = !dungeonGrid.InBounds(glm::ivec2(x, z - 1)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x, z + 1));
			CellType frontCell = !dungeonGrid.InBounds(glm::ivec2(x, z + 1)) ? CellType::None : dungeonGrid.Get(glm::ivec2(x, z - 1));

			if (type == CellType::Hallway) // We are a hallway, we now need to get the cells next to us on the x and z axis
			{
				entities.push_back(PrepareFloor(glm::vec3(xPos, yLevel, zPos), meshScale, glm::vec3(0.4f, 0.0f, 0.0f)));
				/////////////////////////////////
				// BEGIN FLAT HALLWAY CHECKS
				/////////////////////////////////
				if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && backCell == CellType::Hallway && frontCell == CellType::Hallway) // Cross road
				{
					// TODO: Place "cross road" prest
				}

				// Straight hallways by rooms https://gyazo.com/7f8a76c849236390a152c44f1f470c5a
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}

				// Single hallway between 2 rooms https://gyazo.com/4505b05de71bb019253e2b85994417fe
				else if (leftCell == CellType::None && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}

				// Straight hallways by rooms endings https://gyazo.com/56851ec8c824b306bd7ecfbffe0d67b2
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}

				// L-Shaped hallways by room endings https://gyazo.com/69b1c33fcec0444c3f1e109800b9587d
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}

				// Between 2 rooms endings https://gyazo.com/2bb184ca91d15f222146cf66d3b2ed21
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}

				// L-shaped hallways
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					// https://gyazo.com/f7537b1bab7d4c830f3ca7d6bb256428
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					// https://gyazo.com/deb9a057610f8dafab681936910cbfbd
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/31c8d2a173358005c9f9d0cce71a5df6
				}
				else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/01a13d63c6f3abd756e4b021ad2c9747
				}

				// T-Shaped Hallways
				else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				}
				else if(leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				}

				// Straight hallways	
				else if ((leftCell == CellType::Hallway || rightCell == CellType::Hallway) && backCell == CellType::None && frontCell == CellType::None) // Straight hallway going down X
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/710ec568a95561bb22e51a6412a7fb8f
				}
				else if (leftCell == CellType::None && rightCell == CellType::None && (backCell == CellType::Hallway || frontCell == CellType::Hallway)) // Straight hallway going down 
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					// https://gyazo.com/034c62a3becc8e642b2424b1d5985482
				}

				// T-Shaped hallways
				//else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//	// https://gyazo.com/44c213c0dc267decf0332c7c82fa951b
				//}
				//else if (leftCell == CellType::None && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//	// https://gyazo.com/f4ca9244c4cbfccbc7deb845045aa1d2
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				//	// https://gyazo.com/ee6ec031b1b8642848a8afaa110bbb52
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//	// https://gyazo.com/01a13d63c6f3abd756e4b021ad2c9747
				//}

				//https://gyazo.com/f41d71a6a308d82e97ee9a40103a8770
				//else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//
				//}
				//else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}

				//// https://gyazo.com/e772d0ba7a6c032b5e781a669ee96ec5
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}

				//else if (leftCell == CellType::Hallway && rightCell == CellType::None && (frontCell == CellType::Hallway || frontCell == CellType::Room) && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::None && frontCell == CellType::None && (backCell == CellType::Hallway || backCell == CellType::Room))
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}
				//else if ((leftCell == CellType::Hallway || leftCell == CellType::Room) && rightCell == CellType::None && frontCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}

				//else if (leftCell == CellType::None && rightCell == CellType::Hallway && ((frontCell == CellType::Hallway || frontCell == CellType::Room) || (backCell == CellType::Hallway || backCell == CellType::Room)))
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::None && ((frontCell == CellType::Hallway || frontCell == CellType::Room) || (backCell == CellType::Hallway || backCell == CellType::Room)))
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//}
				//else if (((leftCell == CellType::Hallway || leftCell == CellType::Room) || (rightCell == CellType::Hallway || rightCell == CellType::Room)) && frontCell == CellType::None && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//}
				//else if (((leftCell == CellType::Hallway || leftCell == CellType::Room) || (rightCell == CellType::Hallway || rightCell == CellType::Room)) && frontCell == CellType::Hallway && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				//}

				//// Next to rooms
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType:: Room)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				//	// https://gyazo.com/129f6d97bc196e1d4b3c8365e21f41e1
				//}
				//else if (leftCell == CellType::Hallway && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::None)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
				//}
				//else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
				//}
				//else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::Hallway)
				//{
				//	entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
				//}

				/////////////////////////////////
				// END FLAT HALLWAY CHECKS
				/////////////////////////////////
			}
			else if (type == CellType::Room)
			{
				entities.push_back(PrepareFloor(glm::vec3(xPos, yLevel, zPos), meshScale));

				// Room corners
				if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					// https://gyazo.com/e9c68401c1dda0c26491785129b4d876
				}
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					// https://gyazo.com/657260ee9c18ce6e6f8828bb80c3cb1d
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/bfcd278f908f1722733307ec9159e935
				}
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/557e224d05ba153d24816c4c5ffd4ac5
				}

				// Double-Sided Room corners
				if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}

				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}

				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}

				// Room edges
				else if (leftCell == CellType::Room && rightCell == CellType::None && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f)));
					// https://gyazo.com/e549c42921f7da74f9619b85890abf86
				}
				else if (leftCell == CellType::None && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f)));
					// https://gyazo.com/7b26a5d33905789e7a7938099f45f6c8
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::None && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
					// https://gyazo.com/598c8075f235a83685e78eff169329b2
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::None)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale));
					// https://gyazo.com/503ff8144d134cd99bc9c746addffa3d
				}

				// Double-Sided Room edges
				else if (leftCell == CellType::Room && rightCell == CellType::Hallway && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos + wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Hallway && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos - wallOffset, yLevel, zPos), meshScale, glm::quat(0.7071f, 0.0f, -0.7071f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::Hallway && backCell == CellType::Room)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos - wallOffset), meshScale, glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
				else if (leftCell == CellType::Room && rightCell == CellType::Room && frontCell == CellType::Room && backCell == CellType::Hallway)
				{
					entities.push_back(PrepareWall(glm::vec3(xPos, yLevel, zPos + wallOffset), meshScale, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.4f)));
				}
			}
		}
	}

	return entities;
}

void DungeonGenerator2D::GenerateRooms(std::vector<Entity*>& entities)
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

		entities.push_back(PrepareRoom(glm::vec3(newRoom.pos.x, yLevel, newRoom.pos.y), glm::vec3(newRoom.size.x, 1, newRoom.size.y)));
	}
}

void DungeonGenerator2D::Triangulate(std::vector<Entity*>& entities)
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

void DungeonGenerator2D::CreateHallways(std::vector<Entity*>& entities)
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

	for (Edge2D& edge : mst)
	{
		Entity* e = entityManager.PrepareEntity();
		e->AddComponent<PositionComponent>();
		e->AddComponent<ScaleComponent>();
		e->AddComponent<RotationComponent>();
		e->AddComponent<LineRenderComponent>(glm::vec3(edge.a.x, yLevel, edge.a.y), glm::vec3(edge.b.x, yLevel, edge.b.y));
		entities.push_back(e);
	}
}

void DungeonGenerator2D::PathfindHallways(std::vector<Entity*>& entities)
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

Entity* DungeonGenerator2D::PrepareRoom(const glm::ivec3& pos, const glm::ivec3& size)
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

Entity* DungeonGenerator2D::PrepareWall(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot, const glm::vec3& color)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos);
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>(rot);

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = wallMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = color;
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}

Entity* DungeonGenerator2D::PrepareFloor(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& color)
{
	Entity* e = entityManager.PrepareEntity();
	e->AddComponent<PositionComponent>(pos);
	e->AddComponent<ScaleComponent>(scale);
	e->AddComponent<RotationComponent>();

	RenderComponent::RenderInfo renderInfo;
	renderInfo.mesh = floorMesh;
	renderInfo.isColorOverride = true;
	renderInfo.colorOverride = color;
	e->AddComponent<RenderComponent>(renderInfo);
	return e;
}