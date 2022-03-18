#pragma once

#include "Component.h"

class Entity;
class IComponentListener
{
public:
	virtual ~IComponentListener() {}

	virtual void OnAddComponent(Entity* entity, Component* component) = 0;
	virtual void OnRemoveComponent(Entity* entity, Component* component) = 0;
};