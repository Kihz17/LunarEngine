#include "Key.h"

#include <iostream>

Key::Key(int keyCode)
	: keyCode(keyCode),
	down(false),
	justPressed(false)
{

}

Key::~Key()
{

}

void Key::Press()
{
	if (!down)
	{
		down = true;
		justPressed = true;
		timeStamp = glfwGetTime();
	}
	else
	{
		justPressed = false;
	}
}

void Key::Release()
{
	if (down)
	{
		down = false;
		justPressed = true;
		timeStamp = glfwGetTime();
	}
	else
	{
		justPressed = false;
	}
}