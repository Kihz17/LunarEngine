#include "KeyFrameListener.h"

#include <iostream>

void KeyFrameListener::OnKeyFrame(const KeyFrame& frame)
{
	std::cout << "Key frame took place! (Type: " << (int) frame.type << ", Time: " << frame.time << ", Easing Type: " << frame.easingType << ")" << std::endl;
} 