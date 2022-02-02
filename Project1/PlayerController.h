#pragma once

#include "ApplicationLayer.h"
#include "Camera.h"
#include "Entity.h"
#include "Renderer.h"

#include <glm/glm.hpp>

class PlayerController : public ApplicationLayer
{
public:
	PlayerController(Camera& camera, Entity* entity, const WindowSpecs& windowSpecs);
	virtual ~PlayerController();

	virtual void OnUpdate(float deltaTime) override;

private:
	Camera& camera;
	Entity* entity;
	const WindowSpecs& windowSpecs;
	glm::vec2 lastCursorPos;
};