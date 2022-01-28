#include "GLCommon.h"
#include "Renderer.h"
#include "Input.h"
#include "GameEngine.h"
#include "EntityManager.h"
#include "Components.h"
#include "PhysicsFactory.h"
#include "SoundManager.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/imgui/imgui_impl_glfw.h"

#include <glm/gtc/matrix_transform.hpp>

static GameEngine* gameEngine;
GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

glm::vec2 lastCursorPos = glm::vec2(0.0f);

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
    gameEngine->camera.Zoom(yoffset);
}

void SetupWalls(Mesh* plane);
Entity* CreatePhysicsSphere(Mesh* sphereMesh, float radius, const glm::vec3& position, const std::string& name);
float GetRandom(float low, float high);

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
    Mesh* sphere = new Mesh("assets/models/sphere.obj");
    Mesh* plane = new Mesh("assets/models/plane.obj");

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 10.0f, 0.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = EntityManager::CreateEntity("lightTest");
        lightEntity->AddComponent<LightComponent>(light);
    }
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(20.0f, 10.0f, 0.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = EntityManager::CreateEntity("lightTest1");
        lightEntity->AddComponent<LightComponent>(light);
    }
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 10.0f, 20.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = EntityManager::CreateEntity("lightTest2");
        lightEntity->AddComponent<LightComponent>(light);
    }
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(-20.0f, 10.0f, 0.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = EntityManager::CreateEntity("lightTest3");
        lightEntity->AddComponent<LightComponent>(light);
    }
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 10.0f, -20.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = EntityManager::CreateEntity("lightTest4");
        lightEntity->AddComponent<LightComponent>(light);
    }

    // SHADER BALL TEST
    //Entity* testEntity = EntityManager::CreateEntity("shaderBall");
    //testEntity->AddComponent<PositionComponent>();
    //testEntity->AddComponent<RotationComponent>();
    //testEntity->AddComponent<ScaleComponent>();

    //RenderComponent::RenderInfo testInfo;
    //testInfo.vao = shaderBall->GetVertexArray();
    //testInfo.indexCount = shaderBall->GetIndexBuffer()->GetCount();
    //testInfo.albedoTextures.push_back({ blue, 1.0f });
    //testInfo.normalTexture = normalTexture;
    ////testInfo.roughnessTexture = roughnessTexture;
    ////testInfo.metalTexture = metalnessTexture;
    ////testInfo.aoTexture = aoTexture;
    //testEntity->AddComponent<RenderComponent>(testInfo);

    SetupWalls(plane);

    // Initialize physics spheres
    Entity* physicsSpheres[5];
    physicsSpheres[0] = CreatePhysicsSphere(sphere, 0.5f, glm::vec3(0.0f, 5.0f, -20.0f), std::string("physics1"));
    physicsSpheres[1] = CreatePhysicsSphere(sphere, 3.0f, glm::vec3(0.0f, 5.0f, 20.0f), std::string("physics2"));
    physicsSpheres[2] = CreatePhysicsSphere(sphere, 2.0f, glm::vec3(0.0f, 5.0f, 0.0f), std::string("physics3"));
    physicsSpheres[3] = CreatePhysicsSphere(sphere, 1.5f, glm::vec3(20.0f, 5.0f, 0.0f), std::string("physics4"));
    physicsSpheres[4] = CreatePhysicsSphere(sphere, 1.0f, glm::vec3(-20.0f, 5.0f, 0.0f), std::string("physics5"));

    Entity* controlledSphere = physicsSpheres[0];
    controlledSphere->GetComponent<RenderComponent>()->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);

    SoundManager::CreateSound3D("ballCollide", "assets/sounds/hit.mp3", glm::vec3(0.0f, 5.0f, 0.0f));

    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = std::min(currentFrameTime - lastFrameTime, 1.0f);
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // Update camera
        if (Input::GetCursorMode() == CursorMode::Locked)
        {
            RigidBodyComponent* rigidBody = controlledSphere->GetComponent<RigidBodyComponent>();
            const glm::vec3& cameraDir = gameEngine->camera.front;
            if (Input::IsKeyPressed(Key::KeyCode::W))
            {
                rigidBody->ptr->ApplyForce(cameraDir * 20.0f);
                //gameEngine->camera.Move(MoveDirection::Forward, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::A))
            {
                glm::vec3 left = -glm::normalize(glm::cross(cameraDir, gameEngine->camera.worldUp));
                rigidBody->ptr->ApplyForce(left * 20.0f);
                //gameEngine->camera.Move(MoveDirection::Left, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::S))
            {
                rigidBody->ptr->ApplyForce(-cameraDir * 20.0f);
                //gameEngine->camera.Move(MoveDirection::Back, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::D))
            {
                glm::vec3 right = glm::normalize(glm::cross(cameraDir, gameEngine->camera.worldUp));
                rigidBody->ptr->ApplyForce(right * 20.0f);
                //gameEngine->camera.Move(MoveDirection::Right, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::Space))
            {
                rigidBody->ptr->ApplyImpulse(glm::vec3(0.0f, 15.0f, 0.0f));
                //gameEngine->camera.Move(MoveDirection::Up, deltaTime);
            }
            if (Input::IsKeyPressed(Key::KeyCode::LeftShift))
            {
                //gameEngine->camera.Move(MoveDirection::Down, deltaTime);
            }
        }

        // Select sphere
        if (Input::IsKeyPressed(Key::One))
        {
            // Change current sphere back to gray
            RenderComponent* renderComp = controlledSphere->GetComponent<RenderComponent>(); 
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);

            controlledSphere = physicsSpheres[0];

            // Change new sphere to blue
            renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
        }
        else if (Input::IsKeyPressed(Key::Two))
        {
            // Change current sphere back to gray
            RenderComponent* renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);

            controlledSphere = physicsSpheres[1];

            // Change new sphere to blue
            renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
        }
        else if (Input::IsKeyPressed(Key::Three))
        {
            // Change current sphere back to gray
            RenderComponent* renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);

            controlledSphere = physicsSpheres[2];

            // Change new sphere to blue
            renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
        }
        else if (Input::IsKeyPressed(Key::Four))
        {
            // Change current sphere back to gray
            RenderComponent* renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);

            controlledSphere = physicsSpheres[3];

            // Change new sphere to blue
            renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
        }
        else if (Input::IsKeyPressed(Key::Five))
        {
            // Change current sphere back to gray
            RenderComponent* renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);

            controlledSphere = physicsSpheres[4];

            // Change new sphere to blue
            renderComp = controlledSphere->GetComponent<RenderComponent>();
            renderComp->isColorOverride = true;
            renderComp->colorOverride = glm::vec3(0.0f, 0.2f, 0.8f);
        }

        gameEngine->Update(deltaTime);

        const glm::vec3& spherePos = controlledSphere->GetComponent<PositionComponent>()->value;
        gameEngine->camera.position = spherePos -(glm::normalize(gameEngine->camera.front) * 8.0f);

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

void SetupWalls(Mesh* plane)
{
    // GROUND PLANE TEST
    {
        Entity* ground = EntityManager::CreateEntity("ground");
        PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(20.0f));
        RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        rigidInfo.friction = 0.95f;
        RigidBodyComponent* rigidComp = ground->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(0.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
        gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);

        // Render Info
        RenderComponent::RenderInfo groundInfo;
        groundInfo.vao = plane->GetVertexArray();
        groundInfo.indexCount = plane->GetIndexBuffer()->GetCount();
        groundInfo.isColorOverride = true;
        groundInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
        ground->AddComponent<RenderComponent>(groundInfo);
    }

    // Wall 1
    {
        Entity* wall = EntityManager::CreateEntity("wall1");
        PositionComponent* groundPos = wall->AddComponent<PositionComponent>();
        RotationComponent* rotComponent = wall->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = wall->AddComponent<ScaleComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        RigidBodyComponent* rigidComp = wall->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(30.0f, glm::vec3(-1.0f, 0.0f, 0.0f))));
        gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);
    }

    // Wall 2
    {
        Entity* wall = EntityManager::CreateEntity("wall2");
        PositionComponent* groundPos = wall->AddComponent<PositionComponent>();
        RotationComponent* rotComponent = wall->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = wall->AddComponent<ScaleComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        RigidBodyComponent* rigidComp = wall->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(30.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
        gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);
    }

    // Wall 3
    {
        Entity* wall = EntityManager::CreateEntity("wall3");
        PositionComponent* groundPos = wall->AddComponent<PositionComponent>();
        RotationComponent* rotComponent = wall->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = wall->AddComponent<ScaleComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        RigidBodyComponent* rigidComp = wall->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(30.0f, glm::vec3(0.0f, 0.0f, 1.0f))));
        gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);
    }

    // Wall 4
    {
        Entity* wall = EntityManager::CreateEntity("wall4");
        PositionComponent* groundPos = wall->AddComponent<PositionComponent>();
        RotationComponent* rotComponent = wall->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = wall->AddComponent<ScaleComponent>();

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        RigidBodyComponent* rigidComp = wall->AddComponent<RigidBodyComponent>(gameEngine->physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(30.0f, glm::vec3(0.0f, 0.0f, -1.0f))));
        gameEngine->physicsWorld->AddRigidBody(rigidComp->ptr);
    }
}

Entity* CreatePhysicsSphere(Mesh* sphereMesh, float radius, const glm::vec3& position, const std::string& name)
{
    Entity* physicsSphere = EntityManager::CreateEntity(name);
    physicsSphere->AddComponent<PositionComponent>();
    physicsSphere->AddComponent<ScaleComponent>(glm::vec3(radius, radius, radius));
    physicsSphere->AddComponent<RotationComponent>();

    Physics::SphereShape* shape = new Physics::SphereShape(radius);

    // Rigid Body
    Physics::RigidBodyInfo rigidInfo;
    rigidInfo.linearDamping = 0.3f;
    rigidInfo.angularDamping = 0.001f;
    rigidInfo.isStatic = false;
    rigidInfo.mass = radius;
    rigidInfo.position = position;
    rigidInfo.linearVelocity = glm::vec3(0.0f);
    rigidInfo.restitution = 0.8f;
    Physics::IRigidBody* rigidBody = gameEngine->physicsFactory->CreateRigidBody(rigidInfo, shape);
    physicsSphere->AddComponent<RigidBodyComponent>(rigidBody);
    gameEngine->physicsWorld->AddRigidBody(physicsSphere->GetComponent<RigidBodyComponent>()->ptr);

    // Render Info
    RenderComponent::RenderInfo sphereInfo;
    sphereInfo.vao = sphereMesh->GetVertexArray();
    sphereInfo.indexCount = sphereMesh->GetIndexBuffer()->GetCount();
    sphereInfo.isColorOverride = true;
    sphereInfo.colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);
    physicsSphere->AddComponent<RenderComponent>(sphereInfo);

    return physicsSphere;
}

float GetRandom(float low, float high)
{
    return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}