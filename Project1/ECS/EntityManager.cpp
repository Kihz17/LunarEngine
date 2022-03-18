#include "EntityManager.h"
#include "UUID.h"

#include <iostream>

EntityManager::EntityManager()
	: currentEntityID(0)
{

}

EntityManager::~EntityManager()
{
	for (Entity* entity : entities) delete entity;

	entities.clear();

	for (IEntityRemoveListener* removeListener : removeListeners) delete removeListener;
}

const std::vector<Entity*>& EntityManager::GetEntities()
{
	return entities;
}

Entity* EntityManager::CreateEntity(const std::string& name)
{
	unsigned int ID = currentEntityID++;
	Entity* newEntity = new Entity(ID, name);
	entities.push_back(newEntity);
	return newEntity;
}

Entity* EntityManager::CreateEntity()
{
	return CreateEntity(std::to_string(UUID()));
}

Entity* EntityManager::PrepareEntity(const std::string& name)
{
	unsigned int ID = currentEntityID++;
	Entity* newEntity = new Entity(ID, name);
	return newEntity;
}

void EntityManager::ListenToEntity(Entity* e)
{
	entities.push_back(e);
}

void EntityManager::DeleteEntity(Entity* entity)
{
	for (IEntityRemoveListener* removeListener : removeListeners) removeListener->OnEntityRemove(entity);
	entity->valid = false;
	invalidEntities.push_back(entity);
}

void EntityManager::CleanEntities()
{
	for (Entity* e : invalidEntities)
	{
		int eraseIndex = -1;
		for (int i = 0; i < entities.size(); i++)
		{
			if (e == entities[i])
			{
				eraseIndex = i;
				break;
			}
		}
		if (eraseIndex != -1) entities.erase(entities.begin() + eraseIndex);
		delete e;
	}

	invalidEntities.clear();
}