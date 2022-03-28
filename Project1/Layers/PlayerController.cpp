#include "PlayerController.h"
#include "Components.h"
#include "MeshManager.h"
#include "InputManager.h"

#include <iostream>

PlayerController::PlayerController(Camera& camera, EntityManager& entityManager, PhysicsWorld* physicsWorld)
	: entityManager(entityManager),
	physicsWorld(physicsWorld),
	camera(camera)
{

}

void PlayerController::OnAttach()
{
    Mesh* playerMesh = MeshManager::GetMesh("assets/models/sphere.obj");

    playerEntity = entityManager.CreateEntity("Player");
    playerEntity->AddComponent<PositionComponent>(glm::vec3(10.0f, 100.0f, 0.0f));
    playerEntity->AddComponent<RotationComponent>();
    playerEntity->AddComponent<ScaleComponent>(glm::vec3(5.0f, 6.0f, 5.0f));

    RenderComponent::RenderInfo testInfo;
    testInfo.mesh = playerMesh;
    testInfo.isColorOverride = true;
    testInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.6f);
    playerEntity->AddComponent<RenderComponent>(testInfo);
    
    btTransform t;
    t.setOrigin(btVector3(0.0f, 100.0f, 0.0f));

    // Setup player controller ghost object
    btCapsuleShape* capsuleShape = new btCapsuleShape(2.0f, 4.0f);
    ghostObj = new btPairCachingGhostObject();
    ghostObj->setCollisionShape(capsuleShape);
    ghostObj->setCollisionFlags(btCollisionObject::CollisionFlags::CF_CHARACTER_OBJECT);
    ghostObj->setWorldTransform(t);
    physicsWorld->GetBulletWorld()->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback()); // Add default ghost object callback to physics world
    physicsWorld->GetBulletWorld()->addCollisionObject(ghostObj, btCollisionObject::CollisionFlags::CF_CHARACTER_OBJECT, btBroadphaseProxy::CollisionFilterGroups::AllFilter);

    // Setup character controller
    btController = new btKinematicCharacterController(ghostObj, capsuleShape, 5.0f, btVector3(0.0f, 1.0f, 0.0f));
    btController->setGravity(btVector3(0.0f, -100.0f, 0.0f));
    physicsWorld->GetBulletWorld()->addAction(btController); // Add controller to world
}

void PlayerController::OnUpdate(float deltaTime)
{
    glm::vec3 vel(0.0f);
    if (InputManager::GetKey(GLFW_KEY_W)->IsPressed())
    {
        vel += glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));
    }
    else if (InputManager::GetKey(GLFW_KEY_S)->IsPressed())
    {
        vel += -glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));
    }

    if (InputManager::GetKey(GLFW_KEY_A)->IsPressed())
    {
        vel -= glm::normalize(glm::cross(glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z)), glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    else if (InputManager::GetKey(GLFW_KEY_D)->IsPressed())
    {
        vel += glm::normalize(glm::cross(glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z)), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    float speedMult = InputManager::GetKey(GLFW_KEY_LEFT_SHIFT)->IsPressed() ? 0.5f : 0.3f;
    vel *= speedMult;

    if (InputManager::GetKey(GLFW_KEY_SPACE)->IsJustPressed() && btController->onGround())
    {
        btController->jump(btVector3(0.0f, 60.0f, 0.0f));
    }

    btVector3 walkDirection = BulletUtils::GLMVec3ToBullet(vel);
    btController->setWalkDirection(walkDirection);
    btController->updateAction(physicsWorld->GetBulletWorld(), deltaTime);

    glm::vec3 pos = BulletUtils::BulletVec3ToGLM(ghostObj->getWorldTransform().getOrigin());
    camera.position = pos - (camera.front * 40.0f) + glm::vec3(0.0f, 10.0f, 0.0f);
    playerEntity->GetComponent<PositionComponent>()->value = pos;
}