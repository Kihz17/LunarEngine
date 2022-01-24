#pragma once

#include "Entity.h"

#include <unordered_map>

class EntityManager
{
public:

	static void Initialize();
	static void CleanUp();

	static void UpdateEntites(float deltaTime);
	static void RemoveEntity(unsigned int id);

	static const std::unordered_map<unsigned int, Entity*>& GetEntities();
	static Entity* CreateEntity(const std::string& name);

	template<class T, typename... Args> static T* CreateEntity(const std::string& name, Args&&... args)
	{
		T* newEntity = new T(std::forward<Args>(args)...);
		Entity* entity = dynamic_cast<Entity*>(newEntity);
		if (!entity) return nullptr; // Not an instance of entity!

		unsigned int ID = currentEntityID++;
		entities.insert({ ID , newEntity });

		entity->id = ID;
		entity->name = name;

		return newEntity;
	}


private:
	static std::unordered_map<unsigned int, Entity*> entities;
	static unsigned int currentEntityID;
};