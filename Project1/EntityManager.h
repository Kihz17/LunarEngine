#pragma once

#include "Entity.h"

#include <unordered_map>

class EntityManager
{
public:

	static void Initialize();
	static void CleanUp();

	static void RemoveEntity(unsigned int id);

	static const std::unordered_map<unsigned int, Entity*>& GetEntities();
	static Entity* CreateEntity(const std::string& name);


private:
	static std::unordered_map<unsigned int, Entity*> entities;
	static unsigned int currentEntityID;
};