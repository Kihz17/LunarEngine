#include "GLCommon.h"
#include "Renderer.h"
#include "InputManager.h"
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


//static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
//{
//    gameEngine->camera.Zoom(yoffset);
//}

float GetRandom(float low, float high);

void ShaderBallTest(Mesh* shaderBall, Texture* normalTexture, Texture* albedo);
void RenderPipelineKeys(GLFWwindow* window);

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
    glfwSetKeyCallback(window, InputManager::KeyCallback);
    glfwSetCursorPosCallback(window, InputManager::MousePosCallback);
    glfwSetMouseButtonCallback(window, InputManager::MouseKeyCallback);
    glfwSetScrollCallback(window, InputManager::ScrollCallback);

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

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");
    Mesh* sphere = new Mesh("assets/models/sphere.obj");

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 10.0f, 0.0f);
        lightInfo.intensity = 30.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = gameEngine->GetEntityManager().CreateEntity("lightTest");
        lightEntity->AddComponent<LightComponent>(light);
    }

    // SHADER BALL TEST
    ShaderBallTest(shaderBall, normalTexture, blue);

    Key* wKey = InputManager::ListenToKey(GLFW_KEY_W);
    Key* aKey = InputManager::ListenToKey(GLFW_KEY_A);
    Key* sKey = InputManager::ListenToKey(GLFW_KEY_S);
    Key* dKey = InputManager::ListenToKey(GLFW_KEY_D);
    Key* spaceKey = InputManager::ListenToKey(GLFW_KEY_SPACE);
    Key* leftShiftKey = InputManager::ListenToKey(GLFW_KEY_LEFT_SHIFT);

    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = std::min(currentFrameTime - lastFrameTime, 0.1f);
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        RenderPipelineKeys(window);

        // Update camera
        if (InputManager::GetCursorMode() == CursorMode::Locked)
        {
            if (wKey->IsPressed())
            {
                gameEngine->camera.Move(MoveDirection::Forward, deltaTime);
            }
            if (aKey->IsPressed())
            {
                gameEngine->camera.Move(MoveDirection::Left, deltaTime);
            }
            if (sKey->IsPressed())
            {
                gameEngine->camera.Move(MoveDirection::Back, deltaTime);
            }
            if (dKey->IsPressed())
            {
                gameEngine->camera.Move(MoveDirection::Right, deltaTime);
            }
            if (spaceKey->IsPressed())
            {
                gameEngine->camera.Move(MoveDirection::Up, deltaTime);
            }
            if (leftShiftKey->IsPressed())
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

float GetRandom(float low, float high)
{
    return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

void ShaderBallTest(Mesh* shaderBall, Texture* normalTexture, Texture* albedo)
{
    Entity* testEntity = gameEngine->GetEntityManager().CreateEntity("shaderBall");
    testEntity->AddComponent<PositionComponent>();
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>();

    RenderComponent::RenderInfo testInfo;
    testInfo.vao = shaderBall->GetVertexArray();
    testInfo.indexCount = shaderBall->GetIndexBuffer()->GetCount();
    testInfo.albedoTextures.push_back({ albedo, 1.0f });
    testInfo.normalTexture = normalTexture;
    //testInfo.roughnessTexture = roughnessTexture;
    //testInfo.metalTexture = metalnessTexture;
    //testInfo.aoTexture = aoTexture;
    testEntity->AddComponent<RenderComponent>(testInfo);
}

void RenderPipelineKeys(GLFWwindow* window)
{
    Key* escKey = InputManager::ListenToKey(GLFW_KEY_ESCAPE);
    Key* f1Key = InputManager::ListenToKey(GLFW_KEY_F1);
    Key* f2Key = InputManager::ListenToKey(GLFW_KEY_F2);
    Key* f3Key = InputManager::ListenToKey(GLFW_KEY_F3);
    Key* f4Key = InputManager::ListenToKey(GLFW_KEY_F4);
    Key* f5Key = InputManager::ListenToKey(GLFW_KEY_F5);
    Key* f6Key = InputManager::ListenToKey(GLFW_KEY_F6);
    Key* f7Key = InputManager::ListenToKey(GLFW_KEY_F7);
    Key* f8Key = InputManager::ListenToKey(GLFW_KEY_F8);
    Key* f9Key = InputManager::ListenToKey(GLFW_KEY_F9);
    Key* spaceKey = InputManager::ListenToKey(GLFW_KEY_SPACE);

    if (escKey->IsJustPressed())
    {
        CursorMode cursorMode = InputManager::GetCursorMode();
        if (cursorMode == CursorMode::Locked)
        {
            lastCursorPos = glm::vec2(InputManager::GetMouseX(), InputManager::GetMouseY());
            InputManager::SetCursorMode(CursorMode::Normal);
            glfwSetCursorPos(window, WIDTH / 2.0f, HEIGHT / 2.0f);
        }
        else if (cursorMode == CursorMode::Normal)
        {
            glfwSetCursorPos(window, lastCursorPos.x, lastCursorPos.y);
            InputManager::SetCursorMode(CursorMode::Locked);
        }
    }
    
    if (f1Key->IsJustPressed())
    {
        Renderer::SetViewType(1);
    }
    else if (f2Key->IsJustPressed())
    {
        Renderer::SetViewType(2);
    }
    else if (f3Key->IsJustPressed())
    {
        Renderer::SetViewType(3);
    }
    else if (f4Key->IsJustPressed())
    {
        Renderer::SetViewType(4);
    }
    else if (f5Key->IsJustPressed())
    {
        Renderer::SetViewType(5);
    }
    else if (f6Key->IsJustPressed())
    {
        Renderer::SetViewType(6);
    }
    else if (f7Key->IsJustPressed())
    {
        Renderer::SetViewType(7);
    }
    else if (f8Key->IsJustPressed())
    {
        Renderer::SetViewType(8);
    }
    else if (f9Key->IsJustPressed())
    {
        Renderer::SetViewType(9);
    }
}