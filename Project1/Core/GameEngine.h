#pragma once

#include <Physics.h>

#include "GLCommon.h"
#include "ApplicationLayerManager.h"
#include "EntityManager.h"
#include "Window.h"
#include "Mesh.h"
#include "Camera.h"

class GameEngine
{
public:
	GameEngine(const WindowSpecs& windowSpecs, bool editorMode);
	~GameEngine();

	void Run();
	void Stop() { running = false; }

	void Render();

	void AddLayer(ApplicationLayer* layer);
	void AddOverlay(ApplicationLayer* layer);
	void RemoveLayer(ApplicationLayer* layer);
	void RemoveOverlay(ApplicationLayer* layer);

	EntityManager& GetEntityManager() { return entityManager; }

	const WindowSpecs& GetWindowSpecs() const { return windowSpecs; }

	Camera camera;
	Physics::IPhysicsWorld<Entity>* physicsWorld;

	static WindowSpecs InitializeGLFW(bool initImGui);

	bool debugMode;

private:
	void SubmitEntitiesToRender(VertexArrayObject* lineVAO, VertexBuffer* lineVBO, IndexBuffer* lineEBO);

	ApplicationLayerManager layerManager;
	EntityManager entityManager;

	WindowSpecs windowSpecs;

	bool editorMode;

	bool running;
};