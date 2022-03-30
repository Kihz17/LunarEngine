#pragma once

#include "ApplicationLayer.h"
#include "Light.h"
#include "EntityManager.h"

class DayNightCycle : public ApplicationLayer
{
public:
	DayNightCycle(EntityManager& entityManager);

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	EntityManager& entityManager;

	Light* nightLight;
	std::vector<Light*> lanterns;
};