
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

#include "Steering.h"
#include "ApproachShootCondition.h"
#include "SeekCondition.h"

glm::vec2 lastCursorPos = glm::vec2(0.0f);

float GetRandom(float low, float high);

void ShaderBallTest(Mesh* shaderBall, Texture* normalTexture, Texture* albedo, GameEngine& gameEngine);

int main() 
{
    WindowSpecs windowSpecs = GameEngine::InitializeGLFW(true);

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

    GameEngine gameEngine(windowSpecs, true);

    Entity* sphereEnt = gameEngine.SpawnPhysicsSphere("sphere", glm::vec3(0.0f, 5.0f, 0.0f), 1.0f, sphere);
    TagComponent* playerTags = sphereEnt->AddComponent<TagComponent>();
    playerTags->AddTag("player");

    gameEngine.AddLayer(new PlayerController(gameEngine.camera, sphereEnt, gameEngine.GetWindowSpecs()));
    gameEngine.AddLayer(new AnimationLayer(gameEngine.GetEntityManager().GetEntities()));
    gameEngine.AddLayer(new AILayer(gameEngine.GetEntityManager().GetEntities()));

    Entity* behaviourTest = gameEngine.SpawnPhysicsSphere("behaviourTest", glm::vec3(20.0f, 5.0f, 0.0f), 1.0f, sphere);
    SteeringBehaviourComponent* bComp = behaviourTest->AddComponent<SteeringBehaviourComponent>();
    /*bComp->AddTargetingBehaviour(0, new ApproachShootCondition(new SeekBehaviour(behaviourTest->GetComponent<RigidBodyComponent>()->ptr, 
        10.0f, SeekType::Approach, 10.0f), 5.0f));*/
    bComp->AddTargetingBehaviour(0, new SeekCondition(new SeekBehaviour(behaviourTest->GetComponent<RigidBodyComponent>()->ptr, 10.0f, SeekType::None, 10.0f)));

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    // Setup some lights
    {
        LightInfo lightInfo;
        lightInfo.postion = glm::vec3(0.0f, 10.0f, 0.0f);
        lightInfo.intensity = 60.0f;
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
        gameEngine.physicsWorld->AddRigidBody(rigidComp->ptr);

        // Render Info
        RenderComponent::RenderInfo groundInfo;
        groundInfo.vao = plane->GetVertexArray();
        groundInfo.indexCount = plane->GetIndexBuffer()->GetCount();
        groundInfo.isColorOverride = true;
        groundInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
        ground->AddComponent<RenderComponent>(groundInfo);
    }

    //SteeringBehaviourComponent* sComp = sphereEnt->AddComponent<SteeringBehaviourComponent>();
    //SeekBehaviour* seek = new SeekBehaviour(1.0f, 100.0f, 1.0f, 100.0f);
    //FleeBehaviour* flee = new FleeBehaviour(50.0f, 50.0f, 1.0f, 100.0f);
    //sComp->behaviours.push_back(flee);
    //sComp->behaviours.push_back(new WanderBehaviour(1.0f, 1.5f, 10.0f, 10.0f, 10.0f));

    //Entity* testE = gameEngine.GetEntityManager().CreateEntity("camera");
    //PositionComponent* posCompTest = testE->AddComponent<PositionComponent>();
    //{
    //    // Rigid Body
    //    Physics::RigidBodyInfo rigidInfo;
    //    rigidInfo.linearDamping = 0.5f;
    //    rigidInfo.angularDamping = 0.001f;
    //    rigidInfo.isStatic = false;
    //    rigidInfo.mass = 1.0f;
    //    rigidInfo.position = gameEngine.camera.position;
    //    rigidInfo.linearVelocity = glm::vec3(0.0f);
    //    rigidInfo.restitution = 0.8f;
    //    Physics::SphereShape* sh = new Physics::SphereShape(1.0f);
    //    Physics::IRigidBody* rigidBody = gameEngine.physicsFactory->CreateRigidBody(rigidInfo, sh);
    //    testE->AddComponent<RigidBodyComponent>(rigidBody);
    //    gameEngine.physicsWorld->AddRigidBody(testE->GetComponent<RigidBodyComponent>()->ptr);
    //}

    //flee->SetTarget(testE);

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