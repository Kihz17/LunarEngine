#pragma once

#include <string>
#include <vector>

class Component;
class Entity
{
public:
	virtual ~Entity();

	virtual void OnUpdate(float deltaTime);

	template<class T> T* AddComponent()
	{
		T* newComponent = new T();
		assert(newComponent);

		if (dynamic_cast<Component*>(newComponent) == 0) return 0;
		this->components.push_back(newComponent);

		return newComponent;
	}

	template<class T, typename... Args> T* AddComponent(Args&&... args)
	{
		T* newComponent = new T(std::forward<Args>(args)...);
		assert(newComponent);

		if (dynamic_cast<Component*>(newComponent) == 0) return 0;
		assert(!HasComponent<T>());

		this->components.push_back(newComponent);

		return newComponent;
	}

	template<class T> T* GetComponent()
	{
		for (unsigned int i = 0; i < components.size(); i++)
		{
			if (dynamic_cast<T*>(components[i]) != 0)
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
			if (dynamic_cast<T*>(components[i]) != 0) return true;
		}

		return false;
	}

	void RemoveComponent(Component* component);

	const std::vector<Component*>& GetComponents() const { return components; }
	const std::vector<Entity*>& GetChildren() const { return children; }

	unsigned int id;
	std::string name;
	std::vector<Component*> components;
	std::vector<Entity*> children;

protected:
	Entity();

private:
	friend class EntityManager;

	Entity(unsigned int id, const std::string& name);

};

typedef std::vector<Entity*>::iterator entity_iterator;