
#include "Renderer.h"
#include "InputManager.h"
#include "GameEngine.h"
#include "EntityManager.h"
#include "Components.h"
#include "PhysicsFactory.h"
#include "SoundManager.h"
#include "PhysicsWorld.h"

#include "PlayerController.h"
#include "AnimationLayer.h"
#include "AILayer.h"
#include "EnemySpawner.h"

#include "Steering.h"
#include "ApproachShootCondition.h"
#include "SeekCondition.h"
#include "FleeCondition.h"
#include "WanderCondition.h"
#include "IdleCondition.h"
#include "EvadeCondition.h"
#include "PursueCondition.h"
#include "SteeringEntityRemoveListener.h"

glm::vec2 lastCursorPos = glm::vec2(0.0f);

float GetRandom(float low, float high);

void ShaderBallTest(Mesh* shaderBall, Texture* normalTexture, Texture* albedo, GameEngine& gameEngine);

int main() 
{
    // TODO:
    // A way to show "forward" on enemies (gun models?)
    // Eahc type must be different model
    // Each behaviour must be diff color

    WindowSpecs windowSpecs = GameEngine::InitializeGLFW(false);

    // Load models
    Mesh* shaderBall = new Mesh("assets/models/shaderball/shaderball.obj");
    Mesh* sphere = new Mesh("assets/models/sphere.obj");
    Mesh* plane = new Mesh("assets/models/plane.obj");

    // Load textures
    Texture* albedoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* normalTexture = new Texture("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* roughnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* metalnessTexture = new Texture("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* aoTexture = new Texture("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* blue = new Texture("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture* grassTexture = new Texture("assets/textures/grass.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    GameEngine gameEngine(windowSpecs, false);

    Entity* sphereEnt = gameEngine.SpawnPhysicsSphere("sphere", glm::vec3(0.0f, 5.0f, 0.0f), 1.0f, nullptr);
    TagComponent* playerTags = sphereEnt->AddComponent<TagComponent>();
    playerTags->AddTag("player");

    gameEngine.AddLayer(new PlayerController(gameEngine.camera, sphereEnt, gameEngine.GetWindowSpecs(), gameEngine.GetEntityManager(), gameEngine.physicsFactory, gameEngine.physicsWorld, sphere));
    gameEngine.AddLayer(new AnimationLayer(gameEngine.GetEntityManager().GetEntities()));
    AILayer* aiLayer = new AILayer(gameEngine.GetEntityManager().GetEntities());
    gameEngine.AddLayer(aiLayer);
    gameEngine.GetEntityManager().AddEntityRemoveListener(new SteeringEntityRemoveListener(aiLayer));

    gameEngine.AddLayer(new EnemySpawner(3.0f, 40.0f, gameEngine.GetEntityManager(), gameEngine.physicsFactory, gameEngine.physicsWorld, sphere, aiLayer));

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
    //ShaderBallTest(shaderBall, normalTexture, blue);

    {
        Entity* ground = gameEngine.GetEntityManager().CreateEntity("ground");
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

// LATER
// SSAO? Raytracing? Raymarching?
// Post processing

float GetRandom(float low, float high)
{
    return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

void ShaderBallTest(Mesh* shaderBall, Texture* normalTexture, Texture* albedo, GameEngine& gameEngine)
{
    Entity* testEntity = gameEngine.GetEntityManager().CreateEntity("shaderBall");
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