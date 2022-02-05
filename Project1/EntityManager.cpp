#include "EntityManager.h"
#include "UUID.h"

#include <iostream>

EntityManager::EntityManager()
	: currentEntityID(0)
{

}

EntityManager::~EntityManager()
{
	std::unordered_map<unsigned int, Entity*>::iterator it = entities.begin();
	while (it != entities.end())
	{
		delete it->second;
		it++;
	}

	entities.clear();

	for (IEntityRemoveListener* removeListener : removeListeners) delete removeListener;
}


void EntityManager::RemoveEntity(unsigned int id)
{
	std::unordered_map<unsigned int, Entity*>::iterator it = entities.find(id);
	if (it == entities.end()) return;

	DeleteEntity(it->second);
}

const std::unordered_map<unsigned int, Entity*>& EntityManager::GetEntities()
{
	return entities;
}

Entity* EntityManager::CreateEntity(const std::string& name)
{
	unsigned int ID = currentEntityID++;
	Entity* newEntity = new Entity(ID, name);
	entities.insert({ ID , newEntity });
	return newEntity;
}

Entity* EntityManager::CreateEntity()
{
	return CreateEntity(std::to_string(UUID()));
}

void EntityManager::DeleteEntity(Entity* entity)
{
	for (IEntityRemoveListener* removeListener : removeListeners) removeListener->OnEntityRemove(entity);
	entities.erase(entity->id);
	delete entity;
}