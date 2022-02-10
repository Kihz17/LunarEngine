#include "IFrameBuffer.h"

#include <iostream>

namespace FrameBufferConversion
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

	GLenum ConvertOperationToGL(FrameBufferOperationType type)
	{
		switch (type)
		{
		case FrameBufferOperationType::Read:
			return GL_READ_FRAMEBUFFER;
		case FrameBufferOperationType::Write:
			return GL_DRAW_FRAMEBUFFER;
		case FrameBufferOperationType::ReadWrite:
			return GL_FRAMEBUFFER;
		default:
			std::cout << "FrameBufferOperationType did not have a return type!\n";
			return GL_FRAMEBUFFER;
		}
	}
}