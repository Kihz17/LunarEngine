#include "GameEngine.h"

#include "PhysicsFactory.h"
#include "PhysicsWorld.h"
#include "EntityPanel.h"
#include "ShaderLibrary.h"
#include "CollisionListener.h"
#include "SoundManager.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "Renderer.h"

#include "PositionComponent.h"
#include "ScaleComponent.h"
#include "RotationComponent.h"
#include "RigidBodyComponent.h"
#include "SkeletalAnimationComponent.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/imgui/imgui_impl_glfw.h"

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "[ERROR] %d: %s\n", error, description);
}

GameEngine::GameEngine(const WindowSpecs& windowSpecs, bool editorMode)
	: editorMode(editorMode),
    windowSpecs(windowSpecs),
    physicsFactory(new PhysicsFactory()),
    physicsWorld(physicsFactory->CreateWorld()),
    debugMode(false)
{
	// Initialize systems
    InputManager::Initialize(windowSpecs.window);
    TextureManager::Initialize();
	Renderer::Initialize(camera, &this->windowSpecs);
    SoundManager::Initilaize();

    physicsWorld->RegisterCollisionListener(new CollisionListener(this->entityManager));
    physicsWorld->SetGravity(glm::vec3(0.0f, -9.81f, 0.0f));
}

GameEngine::~GameEngine()
{
    for (IPanel* panel : panels) delete panel;

    ShaderLibrary::CleanUp();
    Renderer::CleanUp();
    SoundManager::CleanUp();
    Entity::CleanComponentListeners();
    TextureManager::CleanUp(); // This should be last, to give other things time if they want to remove textures
}

void GameEngine::Render()
{
    // Start ImGui frame
    if (editorMode)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    Renderer::BeginFrame(camera);

    if (editorMode)
    {
        for (IPanel* panel : panels)
        {
            panel->OnUpdate();
        }
        entityPanel.Update(entityManager.GetEntities());
    }

    Renderer::DrawFrame();

    // Render ImGui
    if (editorMode)
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    Renderer::EndFrame();
}

void GameEngine::SubmitEntitiesToRender()
{
    const std::vector<Entity*>& entities = entityManager.GetEntities();
    for (Entity* entity : entities)
    {
        if (entity == nullptr) std::cout << "ge nullEnt\n";
        PositionComponent* posComponent = entity->GetComponent<PositionComponent>();
        if (!posComponent)  continue; // Can't be rendered

        RotationComponent* rotComponent = entity->GetComponent<RotationComponent>();
        if (!rotComponent) continue; // Can't be rendered

        // Sync position component with rigidbody
        RigidBodyComponent* rigidBodyComponent = entity->GetComponent<RigidBodyComponent>();
        if (rigidBodyComponent)
        {
            rigidBodyComponent->ptr->GetPosition(posComponent->value); // Update position component
            rigidBodyComponent->ptr->GetOrientation(rotComponent->value); // Update rotation component
        }

        RenderComponent* renderComponent = entity->GetComponent<RenderComponent>();
        if (!renderComponent) continue; // Can't be rendered

        ScaleComponent* scaleComponent = entity->GetComponent<ScaleComponent>();
        if (!scaleComponent) // Can't be rendered
        {
            std::cout << "Entity '" << entity->name << "' cannot be rendered because it is missing a scale component!" << std::endl;
            continue;
        }

        // Tell the renderer to render this entity
        RenderSubmission submission(renderComponent, posComponent->value, scaleComponent->value, rotComponent->value);

        // Apply bone data to submission if the entity is animated
        SkeletalAnimationComponent* animComp = entity->GetComponent<SkeletalAnimationComponent>();
        if (animComp)
        {

            submission.boneMatrices = animComp->boneMatrices.data();
            submission.boneMatricesLength = animComp->boneMatrices.size();
        }

        Renderer::Submit(submission);

        if (debugMode)
        {
            const AABB* aabb = renderComponent->mesh->GetBoundingBox();
            LineRenderSubmission lineSubmission;
            lineSubmission.vao = aabb->GetVertexArray();
            lineSubmission.indexCount = aabb->GetIndexCount();
            lineSubmission.lineColor = glm::vec3(0.0f, 0.0f, 0.8f);
            lineSubmission.transform = submission.transform;
            Renderer::SubmitLines(lineSubmission);
        }
    }
}

void GameEngine::Run()
{
    this->running = true;

    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;
    while (running)
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = std::min(currentFrameTime - lastFrameTime, 0.1f);
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // Update 3D listener
        ListenerInfo listenerInfo;
        listenerInfo.position = camera.position;
        listenerInfo.velocity = (camera.position - camera.prevPosition) / deltaTime;
        listenerInfo.direction = camera.front;
        listenerInfo.up = camera.up;
        SoundManager::Update(deltaTime, listenerInfo);

        for (ApplicationLayer* layer : this->layerManager)
        {
            layer->OnUpdate(deltaTime);
        }

        physicsWorld->Update(deltaTime);

        // Update camera
        camera.Update(deltaTime);

        InputManager::ClearState();
        entityManager.CleanEntities(); // Remove invalid entities

        // Submit all renderable entities
        SubmitEntitiesToRender();
        Render();
    }

    if (editorMode)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

WindowSpecs GameEngine::InitializeGLFW(bool initImGui)
{
    WindowSpecs windowSpecs;
    windowSpecs.window = nullptr;
    windowSpecs.width = 0;
    windowSpecs.height = 0;
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        std::cout << "Error initializing GLFW!\n";
        return windowSpecs;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* glfwMode = glfwGetVideoMode(glfwMonitor); // contains relevant information about display (width, height, refreshRate, rgb bits)

    // Update window with video mode values
    glfwWindowHint(GLFW_RED_BITS, glfwMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, glfwMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, glfwMode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, glfwMode->refreshRate);

    int WIDTH = glfwMode->width;
    int HEIGHT = glfwMode->height;

    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lunar Engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Set depth function to less than AND equal for skybox depth trick.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Assign callbacks
    glfwSetKeyCallback(window, InputManager::KeyCallback);
    glfwSetCursorPosCallback(window, InputManager::MousePosCallback);
    glfwSetMouseButtonCallback(window, InputManager::MouseKeyCallback);
    glfwSetScrollCallback(window, InputManager::ScrollCallback);

    // Initialize ImGui
    if (initImGui)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 420");
        ImGui::StyleColorsDark();

        // ImGui flags
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    windowSpecs.window = window;
    windowSpecs.width = WIDTH;
    windowSpecs.height = HEIGHT;
    return windowSpecs;
}

void GameEngine::AddLayer(ApplicationLayer* layer)
{
    this->layerManager.AddLayer(layer);
    layer->OnAttach();
}

void GameEngine::AddOverlay(ApplicationLayer* layer)
{
    this->layerManager.AddOverlay(layer);
    layer->OnAttach();
}

void GameEngine::RemoveLayer(ApplicationLayer* layer)
{
    this->layerManager.RemoveLayer(layer, true);
}

void GameEngine::RemoveOverlay(ApplicationLayer* layer)
{
    this->layerManager.RemoveOverlay(layer, true);
}