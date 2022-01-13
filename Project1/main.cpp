#include "GLCommon.h"
#include "Renderer.h"
#include "Input.h"

#include "ScenePanel.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/imgui/imgui_impl_glfw.h"

#include <glm/gtc/matrix_transform.hpp>

GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

Camera camera;

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

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(1);
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(2);
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(3);
    }
    else if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(4);
    }
    else if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(5);
    }
    else if (key == GLFW_KEY_6 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(6);
    }
    else if (key == GLFW_KEY_7 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(7);
    }
    else if (key == GLFW_KEY_8 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(8);
    }
    else if (key == GLFW_KEY_9 && action == GLFW_PRESS)
    {
        Renderer::SetViewType(9);
    }
}

static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (Input::GetCursorMode() == CursorMode::Locked)
    {
        camera.Look(xpos, ypos);
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
    Renderer::Initialize(specs);

    // Load textures
    Texture* albedoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* normalTexture = new Texture("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* roughnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* metalnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* aoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* blue = new Texture("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");
    Mesh* isoSphere = new Mesh("assets/models/ISO_Sphere.ply");
    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    LightInfo lightInfo;
    lightInfo.postion = glm::vec3(0.0f, 5.0f, 0.0f);
    lightInfo.direction = glm::vec3(0.2f, -0.8f, 0.0f);
    lightInfo.attenMode = AttenuationMode::UE4;
  //  Renderer::AddLight("testLight", lightInfo);

    LightInfo lightInfo2;
    lightInfo.postion = glm::vec3(0.0f, -5.0f, 0.0f);
    Renderer::AddLight("testLight2", lightInfo2);

    SubmittedGeometry testGeo;
    testGeo.handle = "test";
    testGeo.vao = shaderBall->GetVertexArray();
    testGeo.indexCount = shaderBall->GetIndexBuffer()->GetCount();
    testGeo.albedoTextures.push_back({ blue, 1.0f });
    testGeo.normalTexture = normalTexture;
    testGeo.roughnessTexture = roughnessTexture;
    testGeo.metalTexture = metalnessTexture;
    testGeo.aoTexture = aoTexture;
    testGeo.hasMaterialTextures = false;

    Renderer::SubmitDefferedGeometry(testGeo);

    // Setup Panels
    std::vector<IPanel*> panels;
    panels.push_back(new ScenePanel());

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
        }

        // Show lights
        if (true)
        {
            std::unordered_map<std::string, Light*> lightMap = Renderer::GetLights();
            std::unordered_map<std::string, Light*>::iterator it = lightMap.begin();
            while (it != lightMap.end())
            {
                Light* light = it->second;

                // Solid center
                ForwardGeometry lightGeo;
                lightGeo.handle = std::string("light" + std::to_string(light->GetIndex()));
                lightGeo.vao = isoSphere->GetVertexArray();
                lightGeo.indexCount = isoSphere->GetIndexBuffer()->GetCount();
                lightGeo.isColorOverride = true;
                lightGeo.colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);
                lightGeo.transform = glm::mat4(1.0f);
                lightGeo.transform *= glm::translate(glm::mat4(1.0f), light->GetPosition());
                Renderer::SubmitForwardGeometry(lightGeo);

                // Wireframe radius
                ForwardGeometry radiusGeo;
                radiusGeo.handle = std::string("lightRadius" + std::to_string(light->GetIndex()));
                radiusGeo.vao = isoSphere->GetVertexArray();
                radiusGeo.indexCount = isoSphere->GetIndexBuffer()->GetCount();
                radiusGeo.isColorOverride = true;
                radiusGeo.colorOverride = glm::vec3(0.8f, 0.0f, 0.0f);
                radiusGeo.isWireframe = true;
                radiusGeo.transform = glm::mat4(1.0f);
                radiusGeo.transform *= glm::translate(glm::mat4(1.0f), light->GetPosition());
                float radius = light->GetRadius();
                radiusGeo.transform *= glm::scale(glm::mat4(1.0f), glm::vec3(radius, radius, radius));
                Renderer::SubmitForwardGeometry(radiusGeo);

                it++;
            }
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Renderer::BeginFrame(camera);

        for (IPanel* panel : panels)
        {
            panel->OnUpdate();
        }

        Renderer::DrawFrame(deltaTime);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Renderer::EndFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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
