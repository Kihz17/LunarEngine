#pragma once

#include <Physics.h>

#include "GLCommon.h"
#include "ApplicationLayerManager.h"
#include "EntityManager.h"
#include "Window.h"
#include "Mesh.h"
#include "EntityPanel.h"
#include "IPanel.h"
#include "Camera.h"

class GameEngine
{
public:
	GameEngine(const WindowSpecs& windowSpecs, bool editorMode);
	~GameEngine();

	void Run();
	void Stop() { running = false; }

	void Render();

	void AddPanel(IPanel* panel) { panels.push_back(panel); }

	void AddLayer(ApplicationLayer* layer);
	void AddOverlay(ApplicationLayer* layer);
	void RemoveLayer(ApplicationLayer* layer);
	void RemoveOverlay(ApplicationLayer* layer);

	EntityManager& GetEntityManager() { return entityManager; }

	const WindowSpecs& GetWindowSpecs() const { return windowSpecs; }

	Camera camera;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;

	static WindowSpecs InitializeGLFW(bool initImGui);

	bool debugMode;

private:
	void SubmitEntitiesToRender();

	ApplicationLayerManager layerManager;
	EntityManager entityManager;

	WindowSpecs windowSpecs;

	EntityPanel entityPanel;
	std::vector<IPanel*> panels;

	bool editorMode;

	bool running;
};