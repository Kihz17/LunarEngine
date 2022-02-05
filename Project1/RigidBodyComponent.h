#pragma once

#include "Component.h"

#include <IRigidBody.h>

struct RigidBodyComponent : public Component
{
	RigidBodyComponent() : ptr(nullptr) {}
	RigidBodyComponent(Physics::IRigidBody* body) : ptr(body) {}

	virtual ~RigidBodyComponent()
	{
		delete ptr;
	}

	virtual void ImGuiUpdate() override
	{

	}

	Physics::IRigidBody* ptr;
};