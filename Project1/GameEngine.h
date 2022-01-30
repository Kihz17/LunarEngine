#pragma once

#include <Physics.h>

#include "EntityManager.h"
#include "AnimationManager.h"

#include "Renderer.h"
#include "EntityPanel.h"
#include "IPanel.h"

class GameEngine
{
public:
	GameEngine(WindowSpecs* window, bool editorMode);
	~GameEngine();

	void Update(float deltaTime);
	void Render();

	void AddPanel(IPanel* panel) { panels.push_back(panel); }

	AnimationManager& GetAnimationManager() { return animationManager; }
	EntityManager& GetEntityManager() { return entityManager; }

	Camera camera;
	Physics::IPhysicsFactory* physicsFactory;
	Physics::IPhysicsWorld* physicsWorld;

private:
	void SubmitEntitiesToRender();

	AnimationManager animationManager;
	EntityManager entityManager;

	WindowSpecs* windowSpecs;

	EntityPanel entityPanel;
	std::vector<IPanel*> panels;

	bool editorMode;

	Mesh* isoSphere;
};