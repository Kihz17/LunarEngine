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

#include "SkeletalAnimationComponentListener.h"
#include "SkeletalAnimationLayer.h"
#include "AnimatedMesh.h"
#include "Animation.h"
#include "DungeonGenerator.h"
#include "LineRenderComponent.h"

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
    Mesh* cube = new Mesh("assets/models/cube.obj");
    Mesh* ship = new Mesh("assets/models/assault.ply");
    Mesh* cyl = new Mesh("assets/models/cylinder.obj");

 /*   AnimatedMesh* vampire = new AnimatedMesh("assets/models/Combat_Model.fbx");
    Animation combo1("assets/models/Combo_1.fbx", vampire);
    Animation combo2("assets/models/Combo_2.fbx", vampire);
    Animation combo3("assets/models/Combo_3.fbx", vampire);
    Animation combo4("assets/models/Combo_4.fbx", vampire);
    Animation combo7("assets/models/Combo_7_Anim.fbx", vampire);*/

    // Load textures
    Texture2D* albedoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* normalTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roughnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* metalnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* aoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blue = TextureManager::CreateTexture2D("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* grassTexture = TextureManager::CreateTexture2D("assets/textures/grass.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodTexture = TextureManager::CreateTexture2D("assets/textures/wood.jpg", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodNormalTexture = TextureManager::CreateTexture2D("assets/textures/woodNormal.jpg", TextureFilterType::Linear, TextureWrapType::Repeat);

    GameEngine gameEngine(windowSpecs, true);
    gameEngine.AddLayer(new FreeCamController(gameEngine.camera, gameEngine.GetWindowSpecs()));

    Renderer::SetEnvironmentMapEquirectangular("assets/textures/hdr/appart.hdr"); // Setup environment map

    gameEngine.camera.position = glm::vec3(0.0f, 10.0f, 30.0f);
    //gameEngine.debugMode = true;

    // Setup some lights    
    LightInfo lightInfo;
    lightInfo.postion = glm::vec3(0.0f, 45.0f, 0.0f);
    lightInfo.intensity = 10.0f;
    Light* light = new Light(lightInfo);
    light->UpdateLightType(LightType::Directional);
    light->UpdateDirection(glm::vec3(0.8f, -0.99f, -0.27f));
    Entity* lightEntity = gameEngine.GetEntityManager().CreateEntity("lightTest");
    lightEntity->AddComponent<LightComponent>(light);

    Renderer::SetShadowMappingDirectionalLight(light);

    // Animation system setup
    SkeletalAnimationLayer* sal = new SkeletalAnimationLayer();
    SkeletalAnimationComponentListener* sacl = new SkeletalAnimationComponentListener(sal->animations);
    Entity::AddComponentListener(sacl);
    gameEngine.AddLayer(sal);

    DungeonGenerator dg(30, glm::ivec3(8, 3, 8), glm::ivec3(100, 50, 100), gameEngine.GetEntityManager());
    std::vector<Entity*> entities = dg.Generate();
    for (Entity* e : entities) gameEngine.GetEntityManager().ListenToEntity(e);

    // SHADER BALL TEST
    //ShaderBallTest(shaderBall, normalTexture, blue, gameEngine);

    //{
    //    Entity* ground = gameEngine.GetEntityManager().CreateEntity("ground");
    //    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(20.0f));
    //    RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
    //    ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(5.0f, 0.1f, 5.0f));

    //    Physics::RigidBodyInfo rigidInfo;
    //    rigidInfo.linearDamping = 0.0f;
    //    rigidInfo.isStatic = true;
    //    rigidInfo.mass = 1.0f;
    //    rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
    //    rigidInfo.linearVelocity = glm::vec3(0.0f);
    //    rigidInfo.friction = 0.95f;
    //    RigidBodyComponent* rigidComp = ground->AddComponent<RigidBodyComponent>(gameEngine.physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(0.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
    //    gameEngine.physicsWorld->AddRigidBody(rigidComp->ptr, ground);

    //    // Render Info
    //    RenderComponent::RenderInfo groundInfo;
    //    groundInfo.mesh = cube;
    //    groundInfo.albedoTextures.push_back(std::make_pair(woodTexture, 1.0f));
    //    groundInfo.normalTexture = woodNormalTexture;
    //    ground->AddComponent<RenderComponent>(groundInfo);
    //}

    //{
    //    Entity* ground = gameEngine.GetEntityManager().CreateEntity("cyl0");
    //    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(30.0f, 5.0f, 3.0f));
    //    RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
    //    ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(4.0f, 10.0f, 4.0f));

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = cyl;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.3f, 0.8f, 0.0f);
    //    renderInfo.reflectRefractMapPriority = ReflectRefractMapPriorityType::Low;
    //    ground->AddComponent<RenderComponent>(renderInfo);
    //}
    //{
    //    Entity* ground = gameEngine.GetEntityManager().CreateEntity("cyl1");
    //    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(-22.0f, 9.0f, -22.0f));
    //    RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
    //    ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(4.0f, 10.0f, 4.0f));

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = cyl;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.4f);
    //    renderInfo.reflectRefractMapPriority = ReflectRefractMapPriorityType::Medium;
    //    ground->AddComponent<RenderComponent>(renderInfo);
    //}
    //{
    //    Entity* ground = gameEngine.GetEntityManager().CreateEntity("cyl2");
    //    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(15.0f, 18.0f, 37.0f));
    //    RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
    //    ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(8.0f, 20.0f, 8.0f));

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = cyl;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.0f, 0.7f, 0.4f);
    //    renderInfo.reflectRefractMapPriority = ReflectRefractMapPriorityType::High;
    //    ground->AddComponent<RenderComponent>(renderInfo);
    //}
    //{
    //    Entity* ground = gameEngine.GetEntityManager().CreateEntity("cyl3");
    //    PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(-25.0f, 13.0f, 27.0f));
    //    RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
    //    ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(6.0f, 15.0f, 6.0f));

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = cyl;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.0f, 0.7f, 0.4f);
    //    renderInfo.reflectRefractMapPriority = ReflectRefractMapPriorityType::High;
    //    ground->AddComponent<RenderComponent>(renderInfo);
    //}

    //{
    //    Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest");
    //    animTest->AddComponent<PositionComponent>(glm::vec3(28.0f, 1.0f, -40.0f));
    //    animTest->AddComponent<ScaleComponent>(glm::vec3(0.05f, 0.05f, 0.05f));
    //    animTest->AddComponent<RotationComponent>();

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = vampire;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
    //    animTest->AddComponent<RenderComponent>(renderInfo);

    //    SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
    //    animComp->SetAnimation(&combo7);
    //    animComp->speed = 10.0f;
    //}

    //{
    //    Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest2");
    //    animTest->AddComponent<PositionComponent>(glm::vec3(18.0f, 1.0f, -40.0f));
    //    animTest->AddComponent<ScaleComponent>(glm::vec3(0.05f, 0.05f, 0.05f));
    //    animTest->AddComponent<RotationComponent>();

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = vampire;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
    //    animTest->AddComponent<RenderComponent>(renderInfo);

    //    SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
    //    animComp->SetAnimation(&combo1);
    //    animComp->speed = 10.0f;
    //}

    //{
    //    Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest3");
    //    animTest->AddComponent<PositionComponent>(glm::vec3(8.0f, 1.0f, -40.0f));
    //    animTest->AddComponent<ScaleComponent>(glm::vec3(0.05f, 0.05f, 0.05f));
    //    animTest->AddComponent<RotationComponent>();

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = vampire;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
    //    animTest->AddComponent<RenderComponent>(renderInfo);

    //    SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
    //    animComp->SetAnimation(&combo2);
    //    animComp->speed = 10.0f;
    //}

    //{
    //    Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest4");
    //    animTest->AddComponent<PositionComponent>(glm::vec3(-8.0f, 1.0f, -40.0f));
    //    animTest->AddComponent<ScaleComponent>(glm::vec3(0.05f, 0.05f, 0.05f));
    //    animTest->AddComponent<RotationComponent>();

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = vampire;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
    //    animTest->AddComponent<RenderComponent>(renderInfo);

    //    SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
    //    animComp->SetAnimation(&combo3);
    //    animComp->speed = 10.0f;
    //}

    //{
    //    Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest5");
    //    animTest->AddComponent<PositionComponent>(glm::vec3(-18.0f, 1.0f, -40.0f));
    //    animTest->AddComponent<ScaleComponent>(glm::vec3(0.05f, 0.05f, 0.05f));
    //    animTest->AddComponent<RotationComponent>();

    //    RenderComponent::RenderInfo renderInfo;
    //    renderInfo.mesh = vampire;
    //    renderInfo.isColorOverride = true;
    //    renderInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
    //    animTest->AddComponent<RenderComponent>(renderInfo);

    //    SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
    //    animComp->SetAnimation(&combo4);
    //    animComp->speed = 10.0f;
    //}

    gameEngine.Run();

    return 0;
}

// TODO List
// 1. Bloom & Emission
// 2. Instanced rendering
// 3. Anti-Aliasing
// 4. Light Probes/Global Illumination
// 5. Water
// 6. Clouds
// 7. Procedural Grass

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
    testEntity->AddComponent<PositionComponent>(glm::vec3(0.0f, 8.0f, 0.0f));
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>();

    RenderComponent::RenderInfo testInfo;
    testInfo.mesh = shaderBall;
    testInfo.albedoTextures.push_back({ albedo, 1.0f });
    testInfo.normalTexture = normalTexture;
    //testInfo.roughnessTexture = roughnessTexture;
    //testInfo.metalTexture = metalnessTexture;
    //testInfo.aoTexture = aoTexture;
    testInfo.reflectRefractType = ReflectRefractType::Reflect;
    testInfo.reflectRefractMapType = ReflectRefractMapType::Environment;
    testInfo.reflectRefractStrength = 0.5f;
    testEntity->AddComponent<RenderComponent>(testInfo);
}