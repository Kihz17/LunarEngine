#include "PlayerController.h"
#include "InputManager.h"
#include "Components.h"

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

float moveSpeed = 50.0f;

PlayerController::PlayerController(Camera& camera, Entity* entity, const WindowSpecs& windowSpecs)
	: camera(camera),
	entity(entity),
    windowSpecs(windowSpecs),
    lastCursorPos(glm::vec2(0.0f, 0.0f))
{

}

PlayerController::~PlayerController()
{

}

void PlayerController::OnUpdate(float deltaTime) 
{
    if (InputManager::GetCursorMode() == CursorMode::Locked)
    {
        camera.Look(InputManager::GetMouseX(), InputManager::GetMouseY()); // Update camera view

        float scroll = InputManager::GetScrollY();
        if (scroll != 0.0) camera.Zoom(scroll);
    }

	Physics::IRigidBody* rigid = entity->GetComponent<RigidBodyComponent>()->ptr;
    rigid->SetOrientation(camera.GetQuaternion()); // Apply camera view to our rotation
	glm::vec3 direction = camera.front;
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

	camera.position = rigid->GetPosition() - (camera.front * 10.0f);

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
