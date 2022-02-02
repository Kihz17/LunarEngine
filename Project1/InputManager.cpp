#include "InputManager.h"

std::unordered_map<int, Key*> InputManager::keys;

double InputManager::mouseX = 0.0;
double InputManager::mouseY = 0.0;
double InputManager::scrollX = 0.0;
double InputManager::scrollY = 0.0;

void InputManager::Initialize()
{

}

Key* InputManager::GetKey(int keyCode)
{
	std::unordered_map<int, Key*>::iterator it = keys.find(keyCode);
	if (it == keys.end()) return nullptr;
	return it->second;
}

Key* InputManager::ListenToKey(int keyCode)
{
	Key* key = GetKey(keyCode);
	if (!key)
	{
		key = new Key(keyCode);
		keys.insert({ keyCode, key });
	}

	return key;
}

void InputManager::ClearState()
{
	std::unordered_map<int, Key*>::iterator it;
	for (it = keys.begin(); it != keys.end(); it++)
	{
		it->second->justPressed = false;
	}

	scrollX = 0.0;
	scrollY = 0.0;
}

void InputManager::SetCursorMode(CursorMode mode)
{
	glfwSetInputMode(static_cast<GLFWwindow*>(glfwGetCurrentContext()), GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
}

CursorMode InputManager::GetCursorMode()
{
	return (CursorMode)(glfwGetInputMode(static_cast<GLFWwindow*>(glfwGetCurrentContext()), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}

void InputManager::KeyCallback(GLFWwindow* window, int keyId, int scancode, int action, int mods)
{
	Key* key = GetKey(keyId);
	if (!key) return;

	if (action == GLFW_PRESS)
	{
		key->Press();
	}
	else if(action == GLFW_RELEASE)
	{
		key->Release();
	}
}

void InputManager::MouseKeyCallback(GLFWwindow* window, int keyId, int action, int mods)
{
	Key* key = GetKey(keyId);
	if (!key) return;

	if (action == GLFW_PRESS)
	{
		key->Press();
	}
	else if(action == GLFW_RELEASE)
	{
		key->Release();
	}
}

void InputManager::MousePosCallback(GLFWwindow* window, double x, double y)
{
	mouseX = x;
	mouseY = y;
}

void InputManager::ScrollCallback(GLFWwindow* window, double x, double y)
{
	scrollX = x;
	scrollY = y;
}