#include "MouseCodes.h"
#include "KeyCodes.h"

#include <map>

enum class CursorMode
{
	Normal = 0,
	Hidden = 1,
	Locked = 2
};

class Input
{
public:
	static bool IsKeyPressed(Key::KeyCode keycode);
	static bool IsMouseButtonPressed(Mouse::MouseCode button);

	static float GetMouseX();
	static float GetMouseY();
	static std::pair<float, float> GetMousePosition();

	static void SetCursorMode(CursorMode mode);
	static CursorMode GetCursorMode();
};