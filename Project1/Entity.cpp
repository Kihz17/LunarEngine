#include "Entity.h"
#include "Components.h"
#include "Renderer.h"

Entity::Entity(unsigned int id, const std::string& name)
	: id(id),
	name(name)
{

}

Entity::~Entity()
{
	for (unsigned int i = 0; i < this->components.size(); i++) RemoveComponent(this->components[i]);
}

void Entity::RemoveComponent(Component* c)
{
	int removeIndex = -1;
	for (unsigned int i = 0; i < components.size(); i++)
	{
		if (this->components[i] == c)
		{
			removeIndex = i;
			break;
		}
	}

	if (removeIndex != -1) components.erase(components.begin() + removeIndex);
}