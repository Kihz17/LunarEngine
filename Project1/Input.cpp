#include "Input.h"

#include "GLCommon.h"

bool Input::IsKeyPressed(Key::KeyCode keycode)
{
	int state = glfwGetKey(glfwGetCurrentContext(), static_cast<int32_t>(keycode));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(Mouse::MouseCode button)
{
	int state = glfwGetMouseButton(static_cast<GLFWwindow*>(glfwGetCurrentContext()), static_cast<int32_t>(button));
	return state == GLFW_PRESS;
}

float Input::GetMouseX()
{
	std::pair<float, float> posPair = GetMousePosition();
	return posPair.first;
}

float Input::GetMouseY()
{
	std::pair<float, float> posPair = GetMousePosition();
	return posPair.second;
}

std::pair<float, float> Input::GetMousePosition()
{
	double x, y;
	glfwGetCursorPos(static_cast<GLFWwindow*>(glfwGetCurrentContext()), &x, &y);
	return { (float)x, (float)y };
}

void Input::SetCursorMode(CursorMode mode)
{
	glfwSetInputMode(static_cast<GLFWwindow*>(glfwGetCurrentContext()), GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
}

CursorMode Input::GetCursorMode()
{
	return (CursorMode)(glfwGetInputMode(static_cast<GLFWwindow*>(glfwGetCurrentContext()), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}