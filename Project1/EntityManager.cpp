#include "EntityManager.h"

#include <iostream>

std::unordered_map<unsigned int, Entity*> EntityManager::entities;
unsigned int EntityManager::currentEntityID = 0;

void EntityManager::Initialize()
{

}

void EntityManager::CleanUp()
{
	std::unordered_map<unsigned int, Entity*>::iterator it = entities.begin();
	while (it != entities.end())
	{
		delete it->second;
		it++;
	}

	entities.clear();
}

void EntityManager::RemoveEntity(unsigned int id)
{
	std::unordered_map<unsigned int, Entity*>::iterator it = entities.find(id);
	if (it == entities.end()) return;

	delete it->second;
	entities.erase(id);
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