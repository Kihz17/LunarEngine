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
    Mesh* rock1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Rock01.FBX");
    Mesh* rock2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Rock02.FBX");
    Mesh* houseFirstFloor1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House1stFloor01.FBX");
    Mesh* houseFirstFloor2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House1stFloor02.FBX");
    Mesh* houseFirstFloor3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House1stFloor03.FBX");
    Mesh* houseFirstFloor4 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House1stFloor04.FBX");
    Mesh* houseFirstFloor5 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House1stFloor05.FBX");
    Mesh* houseSecondFloor1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_House2dFloor01.FBX");
    Mesh* roof1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_HouseRoof01.FBX");
    Mesh* roof2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_HouseRoof02.FBX");
    Mesh* roof3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_HouseRoof03.FBX");
    Mesh* wallBorder6m = MeshManager::GetMesh("assets/models/FantasyVillage/SM_StoneWallBorder6m.FBX");
    Mesh* houseBase1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_HouseBase01.FBX");
    Mesh* castleStairs3m = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleStairs3m.FBX");
    Mesh* castleStairs3mSmall = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleStairs3m01.FBX");
    Mesh* castleBridge6m2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleBridge6m2.FBX");
    Mesh* castleWallCorner6m = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleWallCorner6m01.FBX");
    Mesh* castleWallCorner6m3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleWallCorner3m.FBX");
    Mesh* castleWall6m = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleWall6m.FBX");
    Mesh* chimney3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_ChimneyLarge03.FBX");
    Mesh* barrel1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Barrel01.FBX");
    Mesh* barrel2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Barrel02.FBX");
    Mesh* barrel3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Barrel03.FBX");
    Mesh* barrel4 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Barrel04.FBX");
    Mesh* woodChunks1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodChunks01.FBX");
    Mesh* woodChunks2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodChunks02.FBX");
    Mesh* woodChunks3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodChunks03.FBX");
    Mesh* woodChunks4 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodChunks04.FBX");
    Mesh* woodChunks5 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodChunks05.FBX");
    Mesh* fence1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_WoodFence01.FBX");
    Mesh* wagon1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Wagon01.FBX");
    Mesh* wagon2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Wagon02.FBX");
    Mesh* wagon3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Wagon03.FBX");
    Mesh* canopy1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Canopy01.FBX");
    Mesh* canopy2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Canopy02.FBX");
    Mesh* canopy3 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Canopy03.FBX");
    Mesh* canopy4 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Canopy04.FBX");
    Mesh* canopy5 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Canopy05.FBX");
    Mesh* fabric1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Fabric01.FBX");
    Mesh* fabric2 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Fabric02.FBX");
    Mesh* castleBanner = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Flag01.FBX");
    Mesh* planks = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Planks.FBX");
    Mesh* castleWall3m4 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_CastleWall3m04.FBX");
    Mesh* stoneWallSingle = MeshManager::GetMesh("assets/models/FantasyVillage/SM_StoneWallSingle03.FBX");
    Mesh* door1 = MeshManager::GetMesh("assets/models/FantasyVillage/SM_Door01.FBX");
    Mesh* streetLight = MeshManager::GetMesh("assets/models/FantasyVillage/SM_StreetLight02.FBX");
    Mesh* lantern = MeshManager::GetMesh("assets/models/FantasyVillage/SM_StreetLight03.FBX");

    // Load textures
    Texture2D* albedoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* normalTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roughnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* metalnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* aoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blue = TextureManager::CreateTexture2D("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* terrain = TextureManager::CreateTexture2D("assets/textures/terrain.jpg", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* wood = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodN = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* woodORM = TextureManager::CreateTexture2D("assets/textures/T_WoodDetails_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* groundStoneColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* groundStoneNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* groundStoneORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_GroundStones_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* rockColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Rock01_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* rockNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Rock01_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* rockORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Rock01_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* castleWallColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWall_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* castleWallNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWall_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* castleWallORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWall_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* castleWallDetailColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWallDetails_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* castleWallDetailNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWallDetails_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* castleWallDetailORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_CastleWallDetails_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* houseWallColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_HouseWall_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* houseWallNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_HouseWall_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* houseWallORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_HouseWall_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* roofTilesColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_RoofTiles_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roofTilesGrayColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_RoofTilesGrey_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roofTilesNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_RoofTiles_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roofTilesORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_RoofTiles_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* stoneWallColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneWallNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneWalllORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* fabricColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Fabric01_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* fabricNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Fabric01_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* fabricORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Fabric01_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* doorColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* doorNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* doorORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

    Texture2D* stoneColor = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneNormal = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* stoneORM = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_StoneWall_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);

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
  /*  LightInfo lightInfo;
    lightInfo.postion = glm::vec3(0.0f, 45.0f, 0.0f);
    lightInfo.intensity = 10.0f;
    lightInfo.castShadows = true;
    Light* light = new Light(lightInfo);
    light->UpdateLightType(LightType::Directional);
    light->UpdateDirection(glm::vec3(0.0f, -0.6f, -0.4f));
    Entity* lightEntity = gameEngine.GetEntityManager().CreateEntity("lightTest");
    lightEntity->AddComponent<LightComponent>(light);

    Renderer::SetMainLightSource(light);*/

   /* unsigned int numGrassBlades = 50000;

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

    gameEngine.AddLayer(new EditorLayer(gameEngine.GetEntityManager(), gameEngine.physicsWorld));

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
      /*  if (!e->HasComponent<RigidBodyComponent>()) continue;
        gameEngine.physicsWorld->AddRigidBody(e->GetComponent<RigidBodyComponent>()->ptr, e);*/

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