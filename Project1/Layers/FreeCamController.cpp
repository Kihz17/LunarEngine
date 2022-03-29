#include "FreeCamController.h"
#include "InputManager.h"

FreeCamController::FreeCamController(Camera& camera, const WindowSpecs& windowSpecs)
	: camera(camera),
    lastCursorPos(0.0f, 0.0f),
    windowSpecs(windowSpecs)
{
    wKey = InputManager::GetKey(GLFW_KEY_W);
    aKey = InputManager::GetKey(GLFW_KEY_A);
    sKey = InputManager::GetKey(GLFW_KEY_S);
    dKey = InputManager::GetKey(GLFW_KEY_D);
    spaceKey = InputManager::GetKey(GLFW_KEY_SPACE);
    lShiftKey = InputManager::GetKey(GLFW_KEY_LEFT_SHIFT);
    escKey = InputManager::GetKey(GLFW_KEY_ESCAPE);
    f1Key = InputManager::GetKey(GLFW_KEY_F1);
    f2Key = InputManager::GetKey(GLFW_KEY_F2);
    f3Key = InputManager::GetKey(GLFW_KEY_F3);
    f4Key = InputManager::GetKey(GLFW_KEY_F4);
    f5Key = InputManager::GetKey(GLFW_KEY_F5);
    f6Key = InputManager::GetKey(GLFW_KEY_F6);
    f7Key = InputManager::GetKey(GLFW_KEY_F7);
    f8Key = InputManager::GetKey(GLFW_KEY_F8);
    f9Key = InputManager::GetKey(GLFW_KEY_F9);

    lastCursorPos.x = (float) windowSpecs.width / 2.0f;
    lastCursorPos.y = (float) windowSpecs.height / 2.0f;
    camera.speed = 70.0f;
}

FreeCamController::~FreeCamController()
{

}

void FreeCamController::OnUpdate(float deltaTime)
{
	if (InputManager::GetCursorMode() == CursorMode::Locked)
	{
        camera.Look(InputManager::GetMouseX(), InputManager::GetMouseY()); // Update camera view

        float scroll = InputManager::GetScrollY();
        if (scroll != 0.0) camera.Zoom(scroll);

        glm::vec3 direction = camera.front;
        direction.y = 0.0f;
        direction = glm::normalize(direction);

        if (wKey->IsPressed())
        {
            camera.Move(MoveDirection::Forward, deltaTime);
        }
        else if (sKey->IsPressed())
        {
            camera.Move(MoveDirection::Back, deltaTime);
        }

        if (aKey->IsPressed())
        {
            camera.Move(MoveDirection::Left, deltaTime);
        }
        else if (dKey->IsPressed())
        {
            camera.Move(MoveDirection::Right, deltaTime);
        }

        if (spaceKey->IsPressed())
        {
            camera.Move(MoveDirection::Up, deltaTime);
        }
        else if (lShiftKey->IsPressed())
        {
            camera.Move(MoveDirection::Down, deltaTime);
        }
	}

    if (InputManager::GetKey(GLFW_KEY_P)->IsJustPressed())
    {
        glm::vec3 dir = camera.front;
        std::cout << "Pos: " << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n\n";
        if (glm::abs(dir.x) > glm::abs(dir.y) && glm::abs(dir.x) > glm::abs(dir.z)) // X axis
        {
            std::string s = dir.x < 0 ? "-X" : "X";
            std::cout << "Looking down " << s << " axis.\n";
        }
        else if (glm::abs(dir.z) > glm::abs(dir.y) && glm::abs(dir.z) > glm::abs(dir.x)) // Z axis
        {
            std::string s = dir.z < 0 ? "-Z" : "Z";
            std::cout << "Looking down " << s << " axis.\n";
        }
        else if (glm::abs(dir.y) > glm::abs(dir.x) && glm::abs(dir.y) > glm::abs(dir.z)) // Y axis
        {
            std::string s = dir.y < 0 ? "-Y" : "Y";
            std::cout << "Looking down " << s << " axis.\n";
        }
    }

   // std::cout << "Cam Pos: " << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n";

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