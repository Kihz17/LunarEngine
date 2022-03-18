#pragma once

#include "Entity.h"

class IEntityRemoveListener
{
public:
	virtual ~IEntityRemoveListener() = default;

	virtual void OnEntityRemove(Entity* entity) = 0;
};