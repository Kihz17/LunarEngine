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
	Physics::IPhysicsFactory* physicsFactory;
	Physics::IPhysicsWorld* physicsWorld;

private:
	WindowSpecs* windowSpecs;

	std::vector<IPanel*> panels;

	bool editorMode;

	Mesh* isoSphere;
};