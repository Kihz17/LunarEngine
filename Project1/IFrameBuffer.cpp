#include "IFrameBuffer.h"

#include <iostream>

namespace ColorBufferConversion
{
	GLenum ConvertTypeToGLType(ColorBufferType type)
	{
		switch (type)
		{
		case ColorBufferType::FrontLeft:
			return GL_FRONT_LEFT;
		case ColorBufferType::FrontRight:
			return GL_FRONT_RIGHT;
		case ColorBufferType::BackLeft:
			return GL_BACK_LEFT;
		case ColorBufferType::BackRight:
			return GL_BACK_LEFT;
		case ColorBufferType::Front:
			return GL_FRONT;
		case ColorBufferType::Back:
			return GL_BACK;
		case ColorBufferType::Left:
			return GL_LEFT;
		case ColorBufferType::Right:
			return GL_RIGHT;
		case ColorBufferType::None:
			return GL_NONE;
		default:
			std::cout << "ColorBufferType did not have a return type!\n";
			return GL_RIGHT;
		}
	}
}