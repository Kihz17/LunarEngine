#pragma once

#include "Entity.h"

#include <unordered_map>

class EntityManager
{
public:
	EntityManager();
	virtual ~EntityManager();

	void RemoveEntity(unsigned int id);

	const std::unordered_map<unsigned int, Entity*>& GetEntities();
	Entity* CreateEntity(const std::string& name);

private:
	std::unordered_map<unsigned int, Entity*> entities;
	unsigned int currentEntityID;
};