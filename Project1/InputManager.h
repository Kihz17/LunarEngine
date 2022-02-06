#pragma once

#include "Key.h"

#include <unordered_map>

enum class CursorMode
{
	Normal = 0,
	Hidden = 1,
	Locked = 2
};

class InputManager
{
public:
	static void Initialize(GLFWwindow* window);

	static Key* GetKey(int keyCode);
	static Key* ListenToKey(int keycode);

	static void ClearState();

	static inline double GetMouseX() { return mouseX; }
	static inline double GetMouseY() { return mouseY; }
	static inline double GetScrollX() { return scrollX; }
	static inline double GetScrollY() { return scrollY; }

	static void SetCursorMode(CursorMode mode);
	static CursorMode GetCursorMode();

	static void KeyCallback(GLFWwindow* window, int keyId, int scancode, int action, int mods);
	static void MouseKeyCallback(GLFWwindow* window, int keyId, int action, int mods);
	static void MousePosCallback(GLFWwindow* window, double x, double y);
	static void ScrollCallback(GLFWwindow* window, double x, double y);

private:
	friend class GameEngine;

	static std::unordered_map<int, Key*> keys;

	static double mouseX;
	static double mouseY;
	static double scrollX;
	static double scrollY;

	static GLFWwindow* window;
};