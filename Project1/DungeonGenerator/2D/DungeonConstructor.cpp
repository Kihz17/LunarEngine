#include "DungeonConstructor.h"

namespace DungeonConstructor
{
	bool IsNoneOrRoom(CellType cell)
	{
		return cell == CellType::None || cell == CellType::Room;
	}
}