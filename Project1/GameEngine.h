#pragma once

#include <Physics.h>

#include "Renderer.h"
#include "IPanel.h"

class GameEngine
{
public:
	GameEngine(WindowSpecs* window, bool editorMode);
	~GameEngine();

	void Update(float deltaTime);
	void Render();

	void AddPanel(IPanel* panel) { panels.push_back(panel); }

	Camera camera;

private:
	WindowSpecs* windowSpecs;

	Physics::IPhysicsFactory* physicsFactory;
	Physics::IPhysicsWorld* physicsWorld;

	std::vector<IPanel*> panels;

	bool editorMode;

	Mesh* isoSphere;
};