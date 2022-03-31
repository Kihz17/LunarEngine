#pragma once

#include "IComponentListener.h"

#include <string>
#include <vector>
#include <assert.h>
#include <iostream>

class Entity
{
public:
	virtual ~Entity();

	template<class T> T* AddComponent()
	{
		T* newComponent = new T();
		assert(newComponent);

		if (dynamic_cast<Component*>(newComponent) == 0) return 0;
		this->components.push_back(newComponent);
		for (IComponentListener* listener : componentListeners) listener->OnAddComponent(this, newComponent);

		return newComponent;
	}

	template<class T, typename... Args> T* AddComponent(Args&&... args)
	{
		T* newComponent = new T(std::forward<Args>(args)...);
		assert(newComponent);

		if (dynamic_cast<Component*>(newComponent) == 0) return 0;
		assert(!HasComponent<T>());

		this->components.push_back(newComponent);
		for (IComponentListener* listener : componentListeners) listener->OnAddComponent(this, newComponent);

		return newComponent;
	}

	template<class T> T* GetComponent()
	{
		for (unsigned int i = 0; i < components.size(); i++)
		{
			if (dynamic_cast<T*>(components[i]))
			{
				return dynamic_cast<T*>(components[i]);
			}
		}

		return nullptr;
	}

	template<class T> bool HasComponent()
	{
		for (unsigned int i = 0; i < components.size(); i++)
		{
			if (dynamic_cast<T*>(components[i])) return true;
		}

		return false;
	}

	const std::string& GetName() const { return name; }
	bool IsValid() const { return valid; }

	void RemoveComponent(Component* component);

	const std::vector<Component*>& GetComponents() const { return components; }
	const std::vector<Entity*>& GetChildren() const { return children; }

	static void AddComponentListener(IComponentListener* listener);
	static void CleanComponentListeners();

	unsigned int id;
	std::string name;
	std::vector<Component*> components;
	std::vector<Entity*> children;
	bool shouldSave;

private:
	friend class EntityManager;

	Entity(unsigned int id, const std::string& name);
	bool valid;

	static std::vector<IComponentListener*> componentListeners;
};

typedef std::vector<Entity*>::iterator entity_iterator;