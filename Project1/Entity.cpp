#include "Entity.h"
#include "Components.h"
#include "Renderer.h"

Entity::Entity(unsigned int id, const std::string& name)
	: id(id),
	name(name)
{

}

Entity::Entity()
	: id(0),
	name("unnamed")
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

void Entity::OnUpdate(float deltaTime)
{
	if (HasComponent<RenderComponent>()) // We should be sending to the renderer
	{
		RenderSubmission submission;
		submission.renderComponent = GetComponent<RenderComponent>();

		if (HasComponent<PositionComponent>())
		{
			submission.position = GetComponent<PositionComponent>()->value;
		}

		if (HasComponent<ScaleComponent>())
		{
			submission.scale = GetComponent<ScaleComponent>()->value;
		}

		if (HasComponent<RotationComponent>())
		{
			submission.rotation = GetComponent<RotationComponent>()->value;
		}

		Renderer::Submit(submission);
	}
}