#pragma once

#include <GLFW/glfw3.h>

class Key
{
public:
	virtual ~Key();

	inline bool IsPressed() const { return down; }
	inline bool IsJustPressed() const { return down && justPressed; }
	inline bool IsJustReleased() const { return !down && justPressed; }
	inline double GetElapsedTime() { return glfwGetTime() - timeStamp; } // If key is down: return how many seconds key is down for | If key is up: return how many seconds key is up for 

private:
	friend class InputManager;

	Key(int keyCode);

	void Press();
	void Release();

	bool down;
	bool justPressed;
	double timeStamp;
	int keyCode;
};