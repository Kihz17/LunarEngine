#pragma once

#include "vendor/imgui/imgui.h"

class IPanel
{
public:
	virtual ~IPanel() = default;

	virtual void OnUpdate() = 0;
};