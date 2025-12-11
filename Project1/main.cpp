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
#include "PlayerController.h"
#include "GrassSerializer.h"
#include "DayNightCycle.h"

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
    Mesh* cone = MeshManager::GetMesh("assets/models/cone.obj");
    Mesh* tile4m = MeshManager::GetMesh("assets/models/FantasyVillage/SM_TileGround4m.FBX");

    // Load textures
    Texture2D* wood = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodN = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodORM = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* groundStoneColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* groundStoneNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* groundStoneORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* stoneColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    GameEngine gameEngine(windowSpecs, false);

    // Animation system setup
    SkeletalAnimationLayer* sal = new SkeletalAnimationLayer();
    SkeletalAnimationComponentListener* sacl = new SkeletalAnimationComponentListener(sal->animations);
    Entity::AddComponentListener(sacl);
    gameEngine.AddLayer(sal);

    gameEngine.AddLayer(new FreeCamController(gameEngine.camera, gameEngine.GetWindowSpecs()));

    gameEngine.camera.position = glm::vec3(0.0f, 10.0f, 30.0f);

    //gameEngine.debugMode = true;

    // Setup some lights    
   //LightInfo lightInfo;
   // lightInfo.postion = glm::vec3(0.0f, 45.0f, 0.0f);
   // lightInfo.intensity = 10.0f;
   // lightInfo.castShadows = true;
   // Light* light = new Light(lightInfo);
   // light->UpdateLightType(LightType::Directional);
   // light->UpdateDirection(glm::vec3(0.0f, -0.6f, -0.4f));
   // Entity* lightEntity = gameEngine.GetEntityManager().CreateEntity("lightTest");
   // lightEntity->AddComponent<LightComponent>(light);

   // Renderer::SetMainLightSource(light);

    /*unsigned int numGrassBlades = 50000;

    GrassCluster cluster;
    cluster.grassData.resize(numGrassBlades);
    for (int i = 0; i < numGrassBlades; i++)
    {
        cluster.grassData[i] = glm::vec4(Utils::RandFloat(300.0f, 470.0f), -40.0f, Utils::RandFloat(-150.0f, 200.0f), glm::radians(Utils::RandFloat(0.0f, 360.0f)));
    }

    BufferLayout bufferLayout = {
        { ShaderDataType::Float4, "vWorldPosition" }
    };

    cluster.VAO = new VertexArrayObject();
    cluster.VBO = new VertexBuffer(numGrassBlades * sizeof(glm::vec4));
    cluster.VBO->SetData(cluster.grassData.data(), numGrassBlades * sizeof(glm::vec4));
    cluster.VBO->SetLayout(bufferLayout);
    cluster.VAO->AddVertexBuffer(cluster.VBO);

    cluster.dimensions = glm::vec2(0.4f, 1.2f);
    cluster.discardTexture = TextureManager::CreateTexture2D("assets/textures/grassBladeAlpha.png", TextureFilterType::Linear, TextureWrapType::ClampToEdge);
    cluster.albedoTexture = TextureManager::CreateTexture2D("assets/textures/grassColor.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Renderer::GetGrassClusters().push_back(cluster);*/

    {
        RenderComponent::RenderInfo wallInfo;
        wallInfo.mesh = tile4m;
        wallInfo.albedoTextures.push_back({ stoneColor, 1.0f });
        wallInfo.normalTexture = stoneNormal;
        wallInfo.ormTexture = stoneORM;
        wallInfo.uvOffset = glm::vec2(0.5f, 0.5f);

        RenderComponent::RenderInfo floorInfo;
        floorInfo.mesh = tile4m;
        floorInfo.albedoTextures.push_back({ groundStoneColor, 1.0f });
        floorInfo.normalTexture = groundStoneNormal;
        floorInfo.ormTexture = groundStoneORM;

        RenderComponent::RenderInfo ceilingInfo;
        ceilingInfo.mesh = tile4m;
        ceilingInfo.albedoTextures.push_back({ stoneColor, 1.0f });
        ceilingInfo.normalTexture = stoneNormal;
        ceilingInfo.ormTexture = stoneORM;

        DungeonGenerator2D::DungeonGeneratorInfo dInfo;
        dInfo.roomCount = 30;
        dInfo.minRoomSize = glm::ivec3(3, 2, 3);
        dInfo.maxRoomSize = glm::ivec3(8, 3, 8);
        dInfo.dungeonSize = glm::ivec2(100, 100);
        dInfo.extraPathChance = 0.02f;
        dInfo.yLevel = -50;
        dInfo.dungeonOffset = glm::ivec2(600, -1401);
        dInfo.posScale = 31.2f;
        dInfo.wallOffset = 16.5f;
        dInfo.wallYOffset = 10.0f;
        dInfo.meshScale = glm::vec3(0.039f);
        dInfo.floorRot = glm::quat(-0.7071f, 0.7071f, 0.0f, 0.0f);
        dInfo.wallInfo = wallInfo;
        dInfo.floorInfo = floorInfo;
        dInfo.ceilingInfo = ceilingInfo;

        DungeonGenerator2D dg(dInfo, gameEngine.GetEntityManager());
        dg.Generate();

        glm::vec2 startPos = glm::vec2(600.0f, 40.0f);
        std::vector<Entity*> entities = dg.PlaceEntities(startPos, glm::ivec2(1, 0));

        // Add entities to be rendered
        for (Entity* e : entities)
        {
            e->shouldSave = false;
            e->GetComponent<RenderComponent>()->castShadows = false;
            gameEngine.GetEntityManager().ListenToEntity(e);
        } 
    }

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
    }*/

    // Physics Objects
    {
        Entity* e = gameEngine.GetEntityManager().CreateEntity("PhysicsSphere");
        e->AddComponent<PositionComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>(glm::vec3(2.0f));

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = sphere;
        testInfo.albedoTextures.push_back({ wood, 1.0f });
        testInfo.normalTexture = woodN;
        testInfo.ormTexture = woodORM;
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.mass = 1.0f;
        info.position = glm::vec3(0.0f, 30.0f, 20.0f);
        RigidBody* rb = new RigidBody(info, new Physics::SphereShape(2.0f));
        rb->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddBody(rb);
    }
    {
        Entity* e = gameEngine.GetEntityManager().CreateEntity("PhysicsCube");
        e->AddComponent<PositionComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>(glm::vec3(2.0f));

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = cube;
        testInfo.albedoTextures.push_back({ wood, 1.0f });
        testInfo.normalTexture = woodN;
        testInfo.ormTexture = woodORM;
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.mass = 1.0f;
        info.position = glm::vec3(0.0f, 30.0f, 20.0f);
        RigidBody* rb = new RigidBody(info, new Physics::BoxShape(glm::vec3(2.0f, 2.0f, 2.0f)));
        rb->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddBody(rb);
    }
    {
        Entity* e = gameEngine.GetEntityManager().CreateEntity("PhysicsCone");
        e->AddComponent<PositionComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>(glm::vec3(2.0f));

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = cone;
        testInfo.albedoTextures.push_back({ wood, 1.0f });
        testInfo.normalTexture = woodN;
        testInfo.ormTexture = woodORM;
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.mass = 1.0f;
        info.position = glm::vec3(0.0f, 30.0f, 20.0f);
        RigidBody* rb = new RigidBody(info, new Physics::ConeShape(2.0f, 4.0f));
        rb->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddBody(rb);
    }
    {
        Entity* e = gameEngine.GetEntityManager().CreateEntity("PhysicsCylinder");
        e->AddComponent<PositionComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
        e->AddComponent<RotationComponent>();
        e->AddComponent<ScaleComponent>(glm::vec3(2.0f));

        RenderComponent::RenderInfo testInfo;
        testInfo.mesh = cyl;
        testInfo.albedoTextures.push_back({ wood, 1.0f });
        testInfo.normalTexture = woodN;
        testInfo.ormTexture = woodORM;
        e->AddComponent<RenderComponent>(testInfo);

        Physics::RigidBodyInfo info;
        info.mass = 1.0f;
        info.position = glm::vec3(0.0f, 30.0f, 20.0f);
        RigidBody* rb = new RigidBody(info, new Physics::CylinderShape(glm::vec3(2.0f, 2.0f, 2.0f)));
        rb->GetBulletBody()->setRestitution(0.8f);
        e->AddComponent<RigidBodyComponent>(rb);

        gameEngine.physicsWorld->AddBody(rb);
    }

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
        Renderer::envMap1 = envMap;
    }

    //gameEngine.AddLayer(new EditorLayer(gameEngine.GetEntityManager(), gameEngine.physicsWorld));

    std::ifstream ifs("scene.yaml");
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

        const YAML::Node& grass = root["Grass"];
        if (grass)
        {
            YAML::const_iterator it;
            for (it = grass.begin(); it != grass.end(); it++)
            {
                YAML::Node childNode = (*it);
                GrassCluster g;
                GrassSerializer(g).Deserialize(childNode);
                Renderer::GetGrassClusters().push_back(g);
            }
        }
    }

    for (Entity* e : gameEngine.GetEntityManager().GetEntities())
    {
        if (e->HasComponent<RenderComponent>() && !e->HasComponent<RigidBodyComponent>())
        {
            Physics::RigidBodyInfo info;
            info.mass = 0.0f;
            info.position = e->GetComponent<PositionComponent>()->value;
            info.rotation = e->GetComponent<RotationComponent>()->value;

            Physics::ScaledMeshShape* scaledMeshShape = new Physics::ScaledMeshShape(PhysicsFactory::GetMeshShape(e->GetComponent<RenderComponent>()->mesh), e->GetComponent<ScaleComponent>()->value);
            RigidBody* rb = new RigidBody(info, scaledMeshShape);

            rb->GetBulletBody()->setCollisionFlags(btCollisionObject::CollisionFlags::CF_STATIC_OBJECT);
            e->AddComponent<RigidBodyComponent>(rb);
            gameEngine.physicsWorld->AddBody(rb);
        }
    }

    gameEngine.AddLayer(new PlayerController(gameEngine.camera, gameEngine.GetEntityManager(), static_cast<PhysicsWorld*>(gameEngine.physicsWorld)));
    gameEngine.AddLayer(new DayNightCycle(gameEngine.GetEntityManager()));
    gameEngine.Run();

    return 0;
}

// TODO:
// Attack animations & dodge roll w/ strategic anim cancelleing 
// Fix godrays
// Map details
// Maybe Water??

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