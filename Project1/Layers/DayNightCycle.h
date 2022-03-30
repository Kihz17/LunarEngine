#pragma once

#include "ApplicationLayer.h"
#include "Light.h"

class DayNightCycle : public ApplicationLayer
{
public:
	DayNightCycle();

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	Light* nightLight;
};