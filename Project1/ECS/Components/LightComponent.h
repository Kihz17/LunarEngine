#pragma once

#include "Component.h"
#include "Light.h"

struct LightComponent : public Component
{
	LightComponent() : ptr(nullptr) {}
	LightComponent(Light* ptr) : ptr(ptr) {}
	virtual ~LightComponent()
	{
		delete ptr;
	}

	Light* ptr;
};