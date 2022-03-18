#pragma once

#include "Window.h"
#include "ApplicationLayer.h"
#include "Camera.h"
#include "Renderer.h"
#include "Key.h"

class FreeCamController : public ApplicationLayer
{
public:
	FreeCamController(Camera& camera, const WindowSpecs& windowSpecs);
	virtual ~FreeCamController();

	virtual void OnUpdate(float deltaTime) override;

private:
	Camera& camera;
	glm::vec2 lastCursorPos;
	const WindowSpecs& windowSpecs;

	Key* wKey;
	Key* aKey;
	Key* sKey;
	Key* dKey;
	Key* spaceKey;
	Key* lShiftKey;
	Key* escKey;
	Key* f1Key;
	Key* f2Key;
	Key* f3Key;
	Key* f4Key;
	Key* f5Key;
	Key* f6Key;
	Key* f7Key;
	Key* f8Key;
	Key* f9Key;
};