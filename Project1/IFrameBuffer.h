#pragma once

#include "ITexture.h"
#include "IRenderBuffer.h"

#include <string>

enum class ColorBufferType
{
	FrontLeft,
	FrontRight,

	BackLeft,
	BackRight,

	Front,
	Back,

	Left,
	Right,

	None
};

class IFrameBuffer
{
public:
	virtual ~IFrameBuffer() = default;

	virtual void Bind() const = 0;
	virtual void BindRead() const = 0;
	virtual void BindWrite() const = 0;

	virtual void Unbind() const = 0;
	virtual void UnbindRead() const = 0;
	virtual void UnbindWrite() const = 0;

	virtual void SetColorBufferWrite(ColorBufferType type) const = 0;
	virtual void SetColorBufferWriteAttachment(unsigned int attachment) const = 0;

	virtual void SetColorBufferRead(ColorBufferType type) const = 0;
	virtual void SetColorBufferReadAttachment(unsigned int attachment) const = 0;

	virtual bool CheckComplete() const = 0;

	virtual ITexture* GetColorAttachment(const std::string& name) = 0;
	virtual void AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index) = 0;

	//virtual void SetDepthAttachment() const = 0;
	//virtual void SetStencilAttachment() const = 0;

	virtual void SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const = 0;
};

namespace ColorBufferConversion
{
	GLenum ConvertTypeToGLType(ColorBufferType type);
}