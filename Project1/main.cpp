#include "GameEngine.h"
#include "Window.h"
#include "Mesh.h"

#include "InputManager.h"
#include "EntityManager.h"
#include "TextureManager.h"

#include "Components.h"
#include "PhysicsFactory.h"
#include "PhysicsWorld.h"

#include "FreeCamController.h"
#include "Texture2D.h"

glm::vec2 lastCursorPos = glm::vec2(0.0f);

float GetRandom(float low, float high);

void ShaderBallTest(Mesh* shaderBall, ITexture* normalTexture, ITexture* albedo, GameEngine& gameEngine);

int main() 
{
    WindowSpecs windowSpecs = GameEngine::InitializeGLFW(true);

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");
    Mesh* sphere = new Mesh("assets/models/sphere.obj");
    Mesh* plane = new Mesh("assets/models/plane.obj");

    // Load textures
    Texture2D* albedoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* normalTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roughnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* metalnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* aoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blue = TextureManager::CreateTexture2D("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* grassTexture = TextureManager::CreateTexture2D("assets/textures/grass.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    GameEngine gameEngine(windowSpecs, true);
    gameEngine.AddLayer(new FreeCamController(gameEngine.camera, gameEngine.GetWindowSpecs()));

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 45.0f, 0.0f);
        lightInfo.intensity = 350.0f;
        Light* light = new Light(lightInfo);
        Entity* lightEntity = gameEngine.GetEntityManager().CreateEntity("lightTest");
        lightEntity->AddComponent<LightComponent>(light);
    }

    // SHADER BALL TEST
    ShaderBallTest(shaderBall, normalTexture, blue, gameEngine);

    {
        Entity* ground = gameEngine.GetEntityManager().CreateEntity("ground");
        PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(20.0f));
        RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(2.0f, 1.0f, 2.0f));

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        rigidInfo.friction = 0.95f;
        RigidBodyComponent* rigidComp = ground->AddComponent<RigidBodyComponent>(gameEngine.physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(0.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
        gameEngine.physicsWorld->AddRigidBody(rigidComp->ptr, ground);

        // Render Info
        RenderComponent::RenderInfo groundInfo;
        groundInfo.vao = plane->GetVertexArray();
        groundInfo.indexCount = plane->GetIndexBuffer()->GetCount();
        groundInfo.isColorOverride = true;
        groundInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
        ground->AddComponent<RenderComponent>(groundInfo);
    }

    gameEngine.Run();

	return 0;
}

// TODO List
// 1. Fix IBL
// 2. Instanced rendering
// 3. Prodecural grass
// 4. Look into improving lighting
// 5. Shadow mapping

// LATER
// SSAO? Raytracing? Raymarching?
// Post processing

float GetRandom(float low, float high)
{
    return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

void ShaderBallTest(Mesh* shaderBall, ITexture* normalTexture, ITexture* albedo, GameEngine& gameEngine)
{
    Entity* testEntity = gameEngine.GetEntityManager().CreateEntity("shaderBall");
    testEntity->AddComponent<PositionComponent>(glm::vec3(0.0f, 10.0f, 0.0f));
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