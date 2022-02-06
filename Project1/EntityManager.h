#pragma once

#include "IEntityRemoveListener.h"

#include <unordered_map>
#include <vector>

class EntityManager
{
public:
	EntityManager();
	virtual ~EntityManager();

	const std::vector<Entity*>& GetEntities();
	Entity* CreateEntity(const std::string& name);
	Entity* CreateEntity();

	void AddEntityRemoveListener(IEntityRemoveListener* removeListener) { removeListeners.push_back(removeListener); }
	void DeleteEntity(Entity* entity);
	void CleanEntities();
private:
	std::vector<Entity*> entities;
	unsigned int currentEntityID;

	std::vector<IEntityRemoveListener*> removeListeners;
	std::vector<Entity*> invalidEntities;
};