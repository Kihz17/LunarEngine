#pragma once

#include "IEntityRemoveListener.h"

#include <unordered_map>
#include <vector>

class EntityManager
{
public:
	EntityManager();
	virtual ~EntityManager();

	void RemoveEntity(unsigned int id);

	const std::unordered_map<unsigned int, Entity*>& GetEntities();
	Entity* CreateEntity(const std::string& name);
	Entity* CreateEntity();

	void AddEntityRemoveListener(IEntityRemoveListener* removeListener) { removeListeners.push_back(removeListener); }
	void DeleteEntity(Entity* entity);

private:
	std::unordered_map<unsigned int, Entity*> entities;
	unsigned int currentEntityID;

	std::vector<IEntityRemoveListener*> removeListeners;
};