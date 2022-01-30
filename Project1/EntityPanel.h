#pragma once

#include "IPanel.h"
#include "Entity.h"
 
#include <unordered_map>

class EntityPanel
{
public:
	EntityPanel();
	virtual ~EntityPanel();

	void Update(const std::unordered_map<unsigned int, Entity*>& entities);
};