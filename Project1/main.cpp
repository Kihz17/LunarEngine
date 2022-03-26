#include "GameEngine.h"
#include "Window.h"
#include "MeshManager.h"

#include "InputManager.h"
#include "EntityManager.h"
#include "TextureManager.h"
#include "EquirectangularToCubeMapConverter.h"
#include "PhysicsFactory.h"

#include "Components.h"
#include "PhysicsWorld.h"

#include "FreeCamController.h"
#include "Texture2D.h"

#include "SkeletalAnimationComponentListener.h"
#include "SkeletalAnimationLayer.h"
#include "Animation.h"
#include "DungeonGenerator2D.h"
#include "LineRenderComponent.h"
#include "Utils.h"
#include "ShaderLibrary.h"

#include "EditorLayer.h"
#include "EntitySerializer.h"

#include <fstream>
#include <sstream>

float GetRandom(float low, float high);

void ShaderBallTest(Mesh* shaderBall, ITexture* normalTexture, ITexture* albedo, GameEngine& gameEngine);

int main()
{
    WindowSpecs windowSpecs = GameEngine::InitializeGLFW(true);

    // Load models
    Mesh* shaderBall = MeshManager::GetMesh("assets/models/shaderball/shaderball.obj");
    Mesh* sphere = MeshManager::GetMesh("assets/models/sphere.obj");
    Mesh* plane = MeshManager::GetMesh("assets/models/plane.obj");
    Mesh* cube = MeshManager::GetMesh("assets/models/cube.obj");
    Mesh* cyl = MeshManager::GetMesh("assets/models/cylinder.obj");

    // Load tree
    Mesh* stairs = MeshManager::GetMesh("assets/models/test.fbx");

    // Load textures
    Texture2D* albedoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* normalTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roughnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* metalnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* aoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blue = TextureManager::CreateTexture2D("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodTexture = TextureManager::CreateTexture2D("assets/textures/wood.jpg", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodNormalTexture = TextureManager::CreateTexture2D("assets/textures/woodNormal.jpg", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blueNoiseTexture = TextureManager::CreateTexture2D("assets/textures/BlueNoise.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* wood = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodN = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodORM = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    GameEngine gameEngine(windowSpecs, true);

    // Animation system setup
    SkeletalAnimationLayer* sal = new SkeletalAnimationLayer();
    SkeletalAnimationComponentListener* sacl = new SkeletalAnimationComponentListener(sal->animations);
    Entity::AddComponentListener(sacl);
    gameEngine.AddLayer(sal);

    gameEngine.AddLayer(new FreeCamController(gameEngine.camera, gameEngine.GetWindowSpecs()));

    gameEngine.camera.position = glm::vec3(0.0f, 10.0f, 30.0f);
    //gameEngine.debugMode = true;

    // Setup some lights    
    LightInfo lightInfo;
    lightInfo.postion = glm::vec3(0.0f, 45.0f, 0.0f);
    lightInfo.intensity = 10.0f;
    Light* light = new Light(lightInfo);
    light->UpdateLightType(LightType::Directional);
    light->UpdateDirection(glm::vec3(0.0f, -0.6f, -0.4f));
    Entity* lightEntity = gameEngine.GetEntityManager().CreateEntity("lightTest");
    lightEntity->AddComponent<LightComponent>(light);

    Renderer::SetMainLightSource(light);

    Renderer::GetProceduralGrassCluster().discardTexture = TextureManager::CreateTexture2D("assets/textures/grassBladeAlpha.png", TextureFilterType::Linear, TextureWrapType::ClampToEdge);
    Renderer::GetProceduralGrassCluster().albedoTexture = TextureManager::CreateTexture2D("assets/textures/grassColor.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    //{
    //    DungeonGenerator2D::DungeonGeneratorInfo dInfo;
    //    dInfo.roomCount = 30;
    //    dInfo.minRoomSize = glm::ivec3(3, 2, 3);
    //    dInfo.maxRoomSize = glm::ivec3(8, 3, 8);
    //    dInfo.dungeonSize = glm::ivec2(100, 100);
    //    dInfo.extraPathChance = 0.02f;
    //    dInfo.yLevel = 0;
    //    DungeonGenerator2D dg(dInfo, gameEngine.GetEntityManager());
    //    std::vector<Entity*> entities = dg.Generate();
    //    for (Entity* e : entities) gameEngine.GetEntityManager().ListenToEntity(e);
    //}

    // SHADER BALL TEST
    //ShaderBallTest(shaderBall, normalTexture, blue, gameEngine);
    /*{
        Entity* e = gameEngine.GetEntityManager().CreateEntity();
        e->AddComponent<PositionComponent>();
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>(glm::vec3(100.0f, 0.5f, 100.0f));

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = cube;
        testInfo.isColorOverride = true;
        testInfo.colorOverride = glm::vec3(0.6f, 0.0f, 0.0f);
        testInfo.normalTexture = normalTexture;
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.mass = 0.0f;
        info.initialTransform = glm::mat4(1.0f);
        Physics::IRigidBody* rb = new RigidBody(info, PhysicsFactory::GetMeshScaledShape(testInfo.mesh, glm::vec3(100.0f, 0.5f, 100.0f)));
        dynamic_cast<RigidBody*>(rb)->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddRigidBody(rb, e);
    }
    {
        Entity* e = gameEngine.GetEntityManager().CreateEntity();
        e->AddComponent<PositionComponent>();
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>();

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = sphere;
        testInfo.isColorOverride = true;
        testInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.6f);
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.initialTransform = glm::mat4(1.0f);
        info.initialTransform[3] = glm::vec4(0.0f, 50.0f, 0.0f, 1.0f);
        info.mass = 1.0f;
        info.intertia = glm::vec3(0.5f);
        Physics::IRigidBody* rb = new RigidBody(info, new btSphereShape(1.0f));
        dynamic_cast<RigidBody*>(rb)->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddRigidBody(rb, e);
    }*/

    // Box
    //{
    //    Entity* e = gameEngine.GetEntityManager().CreateEntity();
    //    e->AddComponent<PositionComponent>();
    //    e->AddComponent<RotationComponent>();
    //    e->AddComponent<ScaleComponent>(glm::vec3(1.0f, 1.0f, 1.0f));

    //    RenderComponent::RenderInfo testInfo;
    //    testInfo.mesh = stairs;
    //    testInfo.albedoTextures.push_back({ wood , 1.0f });
    //    testInfo.normalTexture = woodN;
    //    testInfo.ormTexture = woodORM;
    //    e->AddComponent<RenderComponent>(testInfo);
    //}

    // Set env map
    {
        std::vector<std::string> paths;
        paths.push_back("assets/textures/simpleSky.png");
        paths.push_back("assets/textures/simpleSky.png");
        paths.push_back("assets/textures/simpleSky.png");
        paths.push_back("assets/textures/simpleSky.png");
        paths.push_back("assets/textures/simpleSky.png");
        paths.push_back("assets/textures/simpleSky.png");
        CubeMap* envMap = TextureManager::CreateCubeMap(paths, TextureFilterType::Linear, TextureWrapType::Repeat, true, false);
        Renderer::SetEnvironmentMap(envMap);
    }

    gameEngine.AddLayer(new EditorLayer(gameEngine.GetEntityManager(), gameEngine.physicsWorld));

    std::ifstream ifs("test.yaml");
    std::stringstream ss;
    ss << ifs.rdbuf();
    YAML::Node root = YAML::Load(ss.str());
    if (root["Scene"])
    {
        const YAML::Node& entities = root["Entities"];
        if (entities)
        {
            YAML::const_iterator it;
            for (it = entities.begin(); it != entities.end(); it++)
            {
                YAML::Node childNode = (*it);
                EntitySerializer(nullptr, gameEngine.GetEntityManager()).Deserialize(childNode);
            }
        }
    }

    for (Entity* e : gameEngine.GetEntityManager().GetEntities())
    {
        if (!e->HasComponent<RigidBodyComponent>()) continue;
        gameEngine.physicsWorld->AddRigidBody(e->GetComponent<RigidBodyComponent>()->ptr, e);
    }

    gameEngine.Run();

    return 0;
}

// Procedural terrain with the ability to have "holes" in them for entrances to dungeon ]
// Fix godrays
// Maybe Water??

// If we have time, globabl illumination using light probes

float GetRandom(float low, float high)
{
    return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

void ShaderBallTest(Mesh* shaderBall, ITexture* normalTexture, ITexture* albedo, GameEngine& gameEngine)
{
    Entity* testEntity = gameEngine.GetEntityManager().CreateEntity("shaderBall");
    testEntity->AddComponent<PositionComponent>(glm::vec3(0.0f, 4.0f, 0.0f));
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>(glm::vec3(0.5f, 0.5f, 0.5f));

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