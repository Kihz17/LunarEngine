#include "GameEngine.h"

#include "PhysicsFactory.h"
#include "EntityPanel.h"
#include "ShaderLibrary.h"
#include "CollisionListener.h"
#include "SoundManager.h"

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
    SoundManager::Initilaize();

    physicsWorld->RegisterCollisionListener(new CollisionListener());
    physicsWorld->SetGravity(glm::vec3(0.0f, -9.81f, 0.0f));

    if (editorMode)
    {
        
    }
}

GameEngine::~GameEngine()
{
    delete windowSpecs;
	delete physicsWorld;
	delete physicsFactory;
    for (IPanel* panel : panels) delete panel;

    ShaderLibrary::CleanUp();
    Renderer::CleanUp();
    SoundManager::CleanUp();
}

void GameEngine::Update(float deltaTime)
{
    camera.Update(deltaTime);

    animationManager.Update(entityManager.GetEntities(), deltaTime);
    physicsWorld->Update(deltaTime);

    // Update 3D listener
    ListenerInfo listenerInfo;
    listenerInfo.position = camera.position;
    listenerInfo.velocity = (camera.position - camera.prevPosition) / deltaTime;
    listenerInfo.direction = camera.front;
    listenerInfo.up = camera.up;
    SoundManager::Update(deltaTime, listenerInfo);

    SubmitEntitiesToRender();
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
    entityPanel.Update(entityManager.GetEntities());

    Renderer::DrawFrame();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    Renderer::EndFrame();
}

void GameEngine::SubmitEntitiesToRender()
{
    const std::unordered_map<unsigned int, Entity*>& entities = entityManager.GetEntities();
    std::unordered_map<unsigned int, Entity*>::const_iterator it = entities.begin();
    while (it != entities.end())
    {
        Entity* entity = it->second;
        RenderComponent* renderComponent = entity->GetComponent<RenderComponent>();
        if (!renderComponent) // Can't be rendered
        {
            it++;
            continue;
        }

        PositionComponent* posComponent = entity->GetComponent<PositionComponent>();
        if (!posComponent) // Can't be rendered
        {
            std::cout << "Entity '" << entity->name << "' cannot be rendered because it is missing a position component!" << std::endl;
            it++;
            continue;
        }

        RotationComponent* rotComponent = entity->GetComponent<RotationComponent>();
        if (!rotComponent) // Can't be rendered
        {
            std::cout << "Entity '" << entity->name << "' cannot be rendered because it is missing a rotation component!" << std::endl;
            it++;
            continue;
        }

        ScaleComponent* scaleComponent = entity->GetComponent<ScaleComponent>();
        if (!scaleComponent) // Can't be rendered
        {
            std::cout << "Entity '" << entity->name << "' cannot be rendered because it is missing a scale component!" << std::endl;
            it++;
            continue;
        }

        // Sync position component with rigidbody
        RigidBodyComponent* rigidBodyComponent = entity->GetComponent<RigidBodyComponent>();
        if (rigidBodyComponent)
        {
            rigidBodyComponent->ptr->GetPosition(posComponent->value); // Update position component
            rigidBodyComponent->ptr->GetOrientation(rotComponent->value); // Update rotation component
        }

        // Tell the renderer to render this entity
        RenderSubmission submission;
        submission.renderComponent = renderComponent;
        submission.position = posComponent->value;
        submission.rotation = rotComponent->value;
        submission.scale = scaleComponent->value;
        Renderer::Submit(submission);

        it++;
    }
}