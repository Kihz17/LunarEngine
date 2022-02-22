#include "GameEngine.h"
#include "Window.h"
#include "Model.h"

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

glm::vec2 lastCursorPos = glm::vec2(0.0f);

float GetRandom(float low, float high);

void ShaderBallTest(Model* shaderBall, GameEngine& gameEngine);

int main() 
{
    WindowSpecs windowSpecs = GameEngine::InitializeGLFW(true);

    // Load textures
    Texture2D* albedoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_albedo.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* normalTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_normal.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* roughnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_roughness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* metalnessTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_metalness.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* aoTexture = TextureManager::CreateTexture2D("assets/textures/pbr/rustediron/rustediron_ao.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* blue = TextureManager::CreateTexture2D("assets/textures/blue.png", TextureFilterType::Linear, TextureWrapType::Repeat);
    Texture2D* grassTexture = TextureManager::CreateTexture2D("assets/textures/grass.png", TextureFilterType::Linear, TextureWrapType::Repeat);

    // Load models
    Model* shaderBall = new Model("assets/models/shaderball/shaderball.obj");
    {
        Submesh& shaderBallSub = shaderBall->GetSubmeshes()[0];
        shaderBallSub.albedoTextures.push_back({ blue, 1.0f });
        shaderBallSub.normalTexture = normalTexture;
        //shaderBallSub.roughnessTexture = roughnessTexture;
        //shaderBallSub.metalTexture = metalnessTexture;
        //shaderBallSub.aoTexture = aoTexture;
        shaderBallSub.reflectRefractType = ReflectRefractType::Reflect;
        shaderBallSub.reflectRefractMapType = ReflectRefractMapType::Environment;
        shaderBallSub.reflectRefractStrength = 0.5f;
    }


    Model* sphere = new Model("assets/models/sphere.obj");
    Model* plane = new Model("assets/models/plane.obj");
    Model* cube = new Model("assets/models/cube.obj");
    Model* ship = new Model("assets/models/assault.ply");

    Model* vampire = new Model("assets/models/Knight_Golden_Male.fbx");
    {
        for (Submesh& submesh : vampire->GetSubmeshes())
        {
            submesh.isColorOverride = true;
            submesh.colorOverride = glm::vec3(0.8f, 0.0f, 0.0f);
        }
    }
    //Animation anim("assets/models/BaseCharacter.fbx", vampire);

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
    gameEngine.AddLayer(sal);

    // SHADER BALL TEST
    //ShaderBallTest(shaderBall, gameEngine);

    {
        Entity* ground = gameEngine.GetEntityManager().CreateEntity("ground");
        PositionComponent* groundPos = ground->AddComponent<PositionComponent>(glm::vec3(20.0f));
        RotationComponent* rotComponent = ground->AddComponent<RotationComponent>();
        ScaleComponent* scaleComponent = ground->AddComponent<ScaleComponent>(glm::vec3(5.0f, 0.1f, 5.0f));

        Physics::RigidBodyInfo rigidInfo;
        rigidInfo.linearDamping = 0.0f;
        rigidInfo.isStatic = true;
        rigidInfo.mass = 1.0f;
        rigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidInfo.linearVelocity = glm::vec3(0.0f);
        rigidInfo.friction = 0.95f;
        RigidBodyComponent* rigidComp = ground->AddComponent<RigidBodyComponent>(gameEngine.physicsFactory->CreateRigidBody(rigidInfo, new Physics::PlaneShape(0.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
        gameEngine.physicsWorld->AddRigidBody(rigidComp->ptr, ground);

        //// Render Info
        //RenderComponent::RenderInfo groundInfo;
        //groundInfo.mesh = cube;
        //groundInfo.isColorOverride = true;
        //groundInfo.colorOverride = glm::vec3(0.7f, 0.0f, 0.1f);
        //ground->AddComponent<RenderComponent>(groundInfo);
    }

    {
        Entity* animTest = gameEngine.GetEntityManager().CreateEntity("AnimTest");
        animTest->AddComponent<PositionComponent>();
        animTest->AddComponent<ScaleComponent>(glm::vec3(0.1f, 0.1f, 0.1f));
        animTest->AddComponent<RotationComponent>();
        animTest->AddComponent<RenderComponent>(vampire);

        //SkeletalAnimationComponent* animComp = animTest->AddComponent<SkeletalAnimationComponent>();
        //animComp->SetAnimation(&anim);
    }

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

void ShaderBallTest(Model* shaderBall, GameEngine& gameEngine)
{
    Entity* testEntity = gameEngine.GetEntityManager().CreateEntity("shaderBall");
    testEntity->AddComponent<PositionComponent>(glm::vec3(0.0f, 10.0f, 0.0f));
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>();
    testEntity->AddComponent<RenderComponent>(shaderBall);
}