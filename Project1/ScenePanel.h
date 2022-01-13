#pragma once

#include "IPanel.h"

class ScenePanel : public IPanel
{
public:
	ScenePanel();
	virtual ~ScenePanel();

	virtual void OnUpdate() override;
};