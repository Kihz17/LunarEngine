#pragma once

#include "IPanel.h"
#include "Entity.h"
 
#include <vector>

class EntityPanel
{
public:
	EntityPanel();
	virtual ~EntityPanel();

	void Update(const std::vector<Entity*>& entities);
};