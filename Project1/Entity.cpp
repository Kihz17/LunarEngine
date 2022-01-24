#include "Entity.h"

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
	for (unsigned int i = 0; i < components.size(); i++)
	{
		if (this->components[i] == c)
		{
			delete this->components[i];

			if (this->components.size() > 0)
			{
				this->components[i] = this->components[components.size()];
			}

			this->components.pop_back();
			return;
		}
	}
}