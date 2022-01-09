#include "GLCommon.h"
#include "Renderer.h"
#include "Input.h"

GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

Camera camera;

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "[ERROR] %d: %s\n", error, description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        Input::SetCursorMode(Input::GetCursorMode() == CursorMode::Locked ? CursorMode::Normal : CursorMode::Locked);
    }
}

static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    camera.Look(xpos, ypos);
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

    WindowSpecs* specs = new WindowSpecs();
    specs->window = window;
    specs->width = WIDTH;
    specs->height = HEIGHT;
    Renderer::Initialize(specs);

    // Load textures
    Texture* albedoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* normalTexture = new Texture("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* roughnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* metalnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* aoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    Light* l1 = new Light();
    l1->UpdatePosition(glm::vec3(0.0f, 5.0f, 0.0f));
    l1->UpdateDirection(glm::vec3(0.2f, -0.8f, 0.0f));
    l1->UpdateAttenuationMode(AttenuationMode::UE4);

    SubmittedGeometry testGeo;
    testGeo.handle = "test";
    testGeo.mesh = shaderBall;
    testGeo.albedoTextures.push_back({ albedoTexture, 1.0f });
    testGeo.normalTexture = normalTexture;
    testGeo.roughnessTexture = roughnessTexture;
    testGeo.metalTexture = metalnessTexture;
    testGeo.aoTexture = aoTexture;
    testGeo.transform = glm::mat4(1.0f);

    //Renderer::SetViewType(1);

    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // Update camera
        if (Input::IsKeyPressed(Key::KeyCode::W))
        {
            camera.Move(MoveDirection::Forward, deltaTime);
        }
        if (Input::IsKeyPressed(Key::KeyCode::A))
        {
            camera.Move(MoveDirection::Left, deltaTime);
        }
        if (Input::IsKeyPressed(Key::KeyCode::S))
        {
            camera.Move(MoveDirection::Back, deltaTime);
        }
        if (Input::IsKeyPressed(Key::KeyCode::D))
        {
            camera.Move(MoveDirection::Right, deltaTime);
        }
        if (Input::IsKeyPressed(Key::KeyCode::Space))
        {
            camera.Move(MoveDirection::Up, deltaTime);
        }
        if (Input::IsKeyPressed(Key::KeyCode::LeftShift))
        {
            camera.Move(MoveDirection::Down, deltaTime);
        }

        Renderer::SubmitMesh(testGeo);

        Renderer::BeginFrame(camera);

        Renderer::DrawFrame(deltaTime);

        Renderer::EndFrame();
    }

    Renderer::CleanUp();

	return 0;
}

// TODO List
// 1. Fix IBL
// 2. Allow for forward rendering with our new deffered rendering https://learnopengl.com/Advanced-Lighting/Deferred-Shading

// LATER
// SSAO? Raytracing? Raymarching?
// Post processing
// Instanced rendering w/ deffered & forward?
