#include "GameEngine.h"

#include "PhysicsFactory.h"
#include "EntityPanel.h"
#include "EntityManager.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/imgui/imgui_impl_glfw.h"

GameEngine::GameEngine(WindowSpecs* window, bool editorMode)
	: windowSpecs(window),
    physicsFactory(new PhysicsFactory()),
	physicsWorld(physicsFactory->CreateWorld()),
    editorMode(editorMode)
{
	// Initialize systems
	Renderer::Initialize(window);
    EntityManager::Initialize();

    if (editorMode)
    {
        panels.push_back(new EntityPanel());
    }
}

GameEngine::~GameEngine()
{
    delete windowSpecs;
	delete physicsWorld;
	delete physicsFactory;
    for (IPanel* panel : panels) delete panel;

    Renderer::CleanUp();
    EntityManager::CleanUp();
}

void GameEngine::Update(float deltaTime)
{

}

void GameEngine::Render()
{
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Renderer::BeginFrame(camera);

    for (IPanel* panel : panels)
    {
        panel->OnUpdate();
    }

    Renderer::DrawFrame();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    Renderer::EndFrame();
}