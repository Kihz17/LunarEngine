#pragma once

#include "IPanel.h"

class EntityPanel : public IPanel
{
public:
	EntityPanel();
	virtual ~EntityPanel();

	virtual void OnUpdate() override;
};