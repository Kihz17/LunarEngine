#include "GLCommon.h"
#include "Renderer.h"
#include "Input.h"
#include "GameEngine.h"
#include "EntityManager.h"
#include "Components.h"
#include "RigidBody.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/imgui/imgui_impl_glfw.h"

#include <glm/gtc/matrix_transform.hpp>

static GameEngine* gameEngine;
GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

glm::vec2 lastCursorPos = glm::vec2(0.0f);

struct PhysicsSphere : public Entity
{
    PhysicsSphere(float radius)
        : shape(radius),
        rigidBody(nullptr)
    {
        AddComponent<PositionComponent>();
        AddComponent<ScaleComponent>();
        AddComponent<RotationComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.damping = 0.001f;
        rigidInfo.isStatic = false;
        rigidInfo.mass = radius * 1.2;
        rigidInfo.position = GetComponent<PositionComponent>()->value;
        rigidInfo.velocity = glm::vec3(0.0f);
        rigidInfo.restitution = 0.8f;
        rigidBody = new RigidBody(rigidInfo, &shape);

        AddComponent<RigidBodyComponent>(rigidBody);
    }

    ~PhysicsSphere()
    {
        delete rigidBody;
    }

    virtual void OnUpdate(float deltaTime) override
    {
        GetComponent<PositionComponent>()->value = GetComponent<RigidBodyComponent>()->ptr->GetPosition(); // Sync position with rigid body
        Entity::OnUpdate(deltaTime);
    }

private:
    Physics::SphereShape shape;
    RigidBody* rigidBody;
};

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "[ERROR] %d: %s\n", error, description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        CursorMode cursorMode = Input::GetCursorMode();
        if (cursorMode == CursorMode::Locked)
        {
            lastCursorPos = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
            Input::SetCursorMode(CursorMode::Normal);
            glfwSetCursorPos(window, WIDTH / 2.0f, HEIGHT / 2.0f);
        }
        else if (cursorMode == CursorMode::Normal)
        {
            glfwSetCursorPos(window, lastCursorPos.x, lastCursorPos.y);
            Input::SetCursorMode(CursorMode::Locked);
        }
    }

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(1);
    }
    else if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(2);
    }
    else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(3);
    }
    else if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(4);
    }
    else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(5);
    }
    else if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(6);
    }
    else if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(7);
    }
    else if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(8);
    }
    else if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(9);
    }
}

static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (Input::GetCursorMode() == CursorMode::Locked)
    {
        gameEngine->camera.Look(xpos, ypos);
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{

}

int main() 
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        return -1;
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

    WIDTH = glfwMode->width;
    HEIGHT = glfwMode->height;

    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Engine", glfwMonitor, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Set depth function to less than AND equal for skybox depth trick.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Assign callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420");
    ImGui::StyleColorsDark();

    // ImGui flags
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    WindowSpecs* specs = new WindowSpecs();
    specs->window = window;
    specs->width = WIDTH;
    specs->height = HEIGHT;

    gameEngine = new GameEngine(specs, true);

    // Load textures
    Texture* albedoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* normalTexture = new Texture("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* roughnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* metalnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* aoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* blue = new Texture("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* grassTexture = new Texture("assets/textures/grass.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture* grassA = new Texture("assets/textures/Green_Grass_AlbedoA.tga", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* grassN = new Texture("assets/textures/Green_Grass_Normal.tga", TextureFilterType::Linear, TextureWrapType::Repeat);

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");
    Mesh* grass = new Mesh("assets/models/quad.ply");

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    LightInfo lightInfo;
    lightInfo.postion = glm::vec3(0.0f, 10.0f, 0.0f);
    Light* light = new Light(lightInfo);

    Entity* lightEntity = EntityManager::CreateEntity("lightTest");
    lightEntity->AddComponent<LightComponent>(light);

    // SHADER BALL TEST
    Entity* testEntity = EntityManager::CreateEntity("shaderBall");
    testEntity->AddComponent<PositionComponent>();
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>();

    RenderComponent::RenderInfo testInfo;
    testInfo.vao = shaderBall->GetVertexArray();
    testInfo.indexCount = shaderBall->GetIndexBuffer()->GetCount();
    testInfo.albedoTextures.push_back({ blue, 1.0f });
    testInfo.normalTexture = normalTexture;
    //testInfo.roughnessTexture = roughnessTexture;
    //testInfo.metalTexture = metalnessTexture;
    //testInfo.aoTexture = aoTexture;
    testEntity->AddComponent<RenderComponent>(testInfo);

    // GROUND PLANE TEST
    Entity* ground = EntityManager::CreateEntity("ground");
    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(0.0f));

    Physics::RigidBodyInfo rigidInfo;
    rigidInfo.damping = 0.0f;
    rigidInfo.isStatic = true;
    rigidInfo.mass = 1.0f;
    rigidInfo.position = glm::vec3(0.0f);
    rigidInfo.velocity = glm::vec3(0.0f);
    RigidBodyComponent* rigidComp = ground->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(0.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
    gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);

    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // Update camera
        if (Input::GetCursorMode() == CursorMode::Locked)
        {
            if (Input::IsKeyPressed(Key::KeyCode::W))
            {
                gameEngine->camera.Move(MoveDirection::Forward, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::A))
            {
                gameEngine->camera.Move(MoveDirection::Left, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::S))
            {
                gameEngine->camera.Move(MoveDirection::Back, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::D))
            {
                gameEngine->camera.Move(MoveDirection::Right, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::Space))
            {
                gameEngine->camera.Move(MoveDirection::Up, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::LeftShift))
            {
                gameEngine->camera.Move(MoveDirection::Down, deltaTime);
            }
        }

        gameEngine->Update(deltaTime);
        gameEngine->Render();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete gameEngine;

	return 0;
}

// TODO List
// 1. Fix IBL
// 2. Instanced rendering
// 3. Prodecural grass

// LATER
// SSAO? Raytracing? Raymarching?
// Post processing
