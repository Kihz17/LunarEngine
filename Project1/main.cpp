
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
    // Fix EVADE
    // Reset game when player dies
    // enemies spawn at random intervals
    // Add varying health to enemies
    // A way to show "forward" on enemies (gun models?)

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

    Entity* sphereEnt = gameEngine.SpawnPhysicsSphere("sphere", glm::vec3(0.0f, 5.0f, 0.0f), 1.0f, nullptr);
    TagComponent* playerTags = sphereEnt->AddComponent<TagComponent>();
    playerTags->AddTag("player");

    gameEngine.AddLayer(new PlayerController(gameEngine.camera, sphereEnt, gameEngine.GetWindowSpecs(), gameEngine.GetEntityManager(), gameEngine.physicsFactory, gameEngine.physicsWorld, sphere));
    gameEngine.AddLayer(new AnimationLayer(gameEngine.GetEntityManager().GetEntities()));
    AILayer* aiLayer = new AILayer(gameEngine.GetEntityManager().GetEntities());
    gameEngine.AddLayer(aiLayer);
    gameEngine.GetEntityManager().AddEntityRemoveListener(new SteeringEntityRemoveListener(aiLayer));

    // Type A
    //Entity* typeA = gameEngine.SpawnPhysicsSphere("typeA", glm::vec3(20.0f, 5.0f, 0.0f), 1.0f, sphere);
    //SteeringBehaviourComponent* typeAComp = typeA->AddComponent<SteeringBehaviourComponent>();
    //typeAComp->AddTargetingBehaviour(0, new SeekCondition(aiLayer->CreateBehaviour<SeekBehaviour>(typeA->GetComponent<RigidBodyComponent>()->ptr, 1.0f, SeekType::None, false, 10.0f)));
    //typeAComp->AddTargetingBehaviour(1, new FleeCondition(aiLayer->CreateBehaviour<FleeBehaviour>(typeA->GetComponent<RigidBodyComponent>()->ptr, 10.0f)));

    // Type B
    Entity* typeB = gameEngine.SpawnPhysicsSphere("typeB", glm::vec3(10.0f, 5.0f, -20.0f), 1.0f, sphere);
    SteeringBehaviourComponent* typeBComp = typeB->AddComponent<SteeringBehaviourComponent>();
    typeBComp->AddTargetingBehaviour(0, new EvadeCondition(aiLayer->CreateBehaviour<EvadeBehaviour>(typeB->GetComponent<RigidBodyComponent>()->ptr, 1.0f, 60.0f, 1.0f, 100.0f)));
    typeBComp->AddTargetingBehaviour(1, new PursueCondition(aiLayer->CreateBehaviour<PursueBehaviour>(typeB->GetComponent<RigidBodyComponent>()->ptr, 1.0f, 40.0f, 1.0f, 100.0f)));

    // Type C
    //Entity* typeC = gameEngine.SpawnPhysicsSphere("typeC", glm::vec3(0.0f, 5.0f, 20.0f), 1.0f, sphere);
    //SteeringBehaviourComponent* typeCComp = typeC->AddComponent<SteeringBehaviourComponent>();
    //typeCComp->AddTargetingBehaviour(0, new ApproachShootCondition(aiLayer->CreateBehaviour<SeekBehaviour>(typeC->GetComponent<RigidBodyComponent>()->ptr, 10.0f, SeekType::Approach, true, 10.0f), 5.0f));

    //// Type D
    //Entity* typeD = gameEngine.SpawnPhysicsSphere("typeD", glm::vec3(-20.0f, 5.0f, 0.0f), 1.0f, sphere);
    //SteeringBehaviourComponent* typeDComp = typeD->AddComponent<SteeringBehaviourComponent>();
    //typeDComp->AddBehaviour(0, new WanderCondition(aiLayer->CreateBehaviour<WanderBehaviour>(typeD->GetComponent<RigidBodyComponent>()->ptr, 20.0f, 10.0f, 10.0f), 6.0f, 3.0f));
    //typeDComp->AddBehaviour(1, new IdleCondition(aiLayer->CreateBehaviour<IdleBehaviour>(typeD->GetComponent<RigidBodyComponent>()->ptr), 3.0f, 6.0f));

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