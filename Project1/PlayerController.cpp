#include "PlayerController.h"
#include "InputManager.h"
#include "Components.h"
#include "UUID.h"

#include <Shapes.h>

Key* wKey = InputManager::ListenToKey(GLFW_KEY_W);
Key* aKey = InputManager::ListenToKey(GLFW_KEY_A);
Key* sKey = InputManager::ListenToKey(GLFW_KEY_S);
Key* dKey = InputManager::ListenToKey(GLFW_KEY_D);
Key* escKey = InputManager::ListenToKey(GLFW_KEY_ESCAPE);
Key* f1Key = InputManager::ListenToKey(GLFW_KEY_F1);
Key* f2Key = InputManager::ListenToKey(GLFW_KEY_F2);
Key* f3Key = InputManager::ListenToKey(GLFW_KEY_F3);
Key* f4Key = InputManager::ListenToKey(GLFW_KEY_F4);
Key* f5Key = InputManager::ListenToKey(GLFW_KEY_F5);
Key* f6Key = InputManager::ListenToKey(GLFW_KEY_F6);
Key* f7Key = InputManager::ListenToKey(GLFW_KEY_F7);
Key* f8Key = InputManager::ListenToKey(GLFW_KEY_F8);
Key* f9Key = InputManager::ListenToKey(GLFW_KEY_F9);
Key* lmbKey = InputManager::ListenToKey(GLFW_MOUSE_BUTTON_LEFT);

float moveSpeed = 50.0f;

PlayerController::PlayerController(Camera& camera, Entity* entity, const WindowSpecs& windowSpecs, EntityManager& entityManager, 
    Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, Mesh* bulletMesh)
	: camera(camera),
	entity(entity),
    windowSpecs(windowSpecs),
    lastCursorPos(glm::vec2(0.0f, 0.0f)),
    entityManager(entityManager),
    physicsFactory(physicsFactory),
    physicsWorld(physicsWorld),
    bulletMesh(bulletMesh)
{

}

PlayerController::~PlayerController()
{

}

void PlayerController::OnUpdate(float deltaTime) 
{
    Physics::IRigidBody* rigid = entity->GetComponent<RigidBodyComponent>()->ptr;

    if (InputManager::GetCursorMode() == CursorMode::Locked)
    {
        camera.Look(InputManager::GetMouseX(), InputManager::GetMouseY()); // Update camera view

        float scroll = InputManager::GetScrollY();
        if (scroll != 0.0) camera.Zoom(scroll);

        rigid->SetOrientation(camera.GetQuaternion()); // Apply camera view to our rotation
	    glm::vec3 direction = camera.front;
        direction.y = 0.0f;
        direction = glm::normalize(direction);

	    if (wKey->IsPressed())
	    {
		    rigid->ApplyForce(direction * moveSpeed);
	    }
	    else if (sKey->IsPressed())
	    {
		    rigid->ApplyForce(direction * -moveSpeed);
	    }

	    if (aKey->IsPressed())
	    {
		    glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.worldUp));
		    rigid->ApplyForce(-right * moveSpeed);
	    }
	    else if (dKey->IsPressed())
	    {
		    glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.worldUp));
		    rigid->ApplyForce(right * moveSpeed);
	    }

        if (lmbKey->IsJustPressed())
        {
            SpawnBullet(camera.position, camera.front);
        }
    }

    camera.position = rigid->GetPosition();
    camera.position.y += 2.0f;

    if (escKey->IsJustPressed())
    {
        CursorMode cursorMode = InputManager::GetCursorMode();
        if (cursorMode == CursorMode::Locked)
        {
            lastCursorPos = glm::vec2(InputManager::GetMouseX(), InputManager::GetMouseY());
            InputManager::SetCursorMode(CursorMode::Normal);
            glfwSetCursorPos(windowSpecs.window, windowSpecs.width / 2.0f, windowSpecs.height / 2.0f);
        }
        else if (cursorMode == CursorMode::Normal)
        {
            glfwSetCursorPos(windowSpecs.window, lastCursorPos.x, lastCursorPos.y);
            InputManager::SetCursorMode(CursorMode::Locked);
        }
    }

    if (f1Key->IsJustPressed())
    {
        Renderer::SetViewType(1);
    }
    else if (f2Key->IsJustPressed())
    {
        Renderer::SetViewType(2);
    }
    else if (f3Key->IsJustPressed())
    {
        Renderer::SetViewType(3);
    }
    else if (f4Key->IsJustPressed())
    {
        Renderer::SetViewType(4);
    }
    else if (f5Key->IsJustPressed())
    {
        Renderer::SetViewType(5);
    }
    else if (f6Key->IsJustPressed())
    {
        Renderer::SetViewType(6);
    }
    else if (f7Key->IsJustPressed())
    {
        Renderer::SetViewType(7);
    }
    else if (f8Key->IsJustPressed())
    {
        Renderer::SetViewType(8);
    }
    else if (f9Key->IsJustPressed())
    {
        Renderer::SetViewType(9);
    }
}

Entity* PlayerController::SpawnBullet(const glm::vec3 & position, const glm::vec3 & direction)
{
    constexpr float bulletRadius = 0.2f;
    constexpr float bulletSpeed = 40.0f;
    Entity* physicsSphere = entityManager.CreateEntity(std::to_string(UUID()));
    physicsSphere->AddComponent<PositionComponent>();
    physicsSphere->AddComponent<ScaleComponent>(glm::vec3(bulletRadius, bulletRadius, bulletRadius));
    physicsSphere->AddComponent<RotationComponent>();
    TagComponent* tagComp = physicsSphere->AddComponent<TagComponent>();
    tagComp->AddTag("bullet");

    Physics::SphereShape* shape = new Physics::SphereShape(bulletRadius);

    // Rigid Body
    Physics::RigidBodyInfo rigidInfo;
    rigidInfo.linearDamping = 0.001f;
    rigidInfo.angularDamping = 0.001f;
    rigidInfo.isStatic = false;
    rigidInfo.mass = 0.01f;
    rigidInfo.position = position;
    rigidInfo.linearVelocity = direction * bulletSpeed;
    rigidInfo.restitution = 0.001f;
    Physics::IRigidBody* rigidBody = physicsFactory->CreateRigidBody(rigidInfo, shape);
    rigidBody->UseLocalGravity(true);
    rigidBody->SetGravityAcceleration(glm::vec3(0.0f, -2.5f, 0.0f));
    physicsSphere->AddComponent<RigidBodyComponent>(rigidBody);
    physicsWorld->AddRigidBody(physicsSphere->GetComponent<RigidBodyComponent>()->ptr, physicsSphere);

    // Render Info
    RenderComponent::RenderInfo sphereInfo;
    sphereInfo.vao = bulletMesh->GetVertexArray();
    sphereInfo.indexCount = bulletMesh->GetIndexBuffer()->GetCount();
    sphereInfo.isColorOverride = true;
    sphereInfo.colorOverride = glm::vec3(0.8f, 0.8f, 0.8f);
    physicsSphere->AddComponent<RenderComponent>(sphereInfo);
    return physicsSphere;
}