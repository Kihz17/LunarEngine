#pragma once

#include "ITexture.h"
#include "IRenderBuffer.h"

#include <string>
#include <glm/glm.hpp>

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

enum class FrameBufferOperationType
{
	Read,
	Write,
	ReadWrite
};

enum class CubeMapFace
{
	PosX = 0,
	NegX,
	PosY,
	NegY,
	PosZ,
	NegZ
};

class CubeMap;
class Texture2D;
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
	virtual void AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite) = 0;
	virtual void AddColorAttachmentCubeMapFace(const std::string& name, CubeMap* texture, unsigned int index, CubeMapFace face, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite) = 0;

	virtual void SetDepthAttachment(ITexture* texture, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite, int level = 0) = 0;
	virtual void SetDepthAttachment2D(Texture2D* texture, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite, int level = 0) = 0;
	//virtual void SetStencilAttachment() const = 0;

	virtual void SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const = 0;

	virtual void ClearColorBuffer(const glm::vec4& value, unsigned int index = 0) const = 0;
};

namespace FrameBufferConversion
{
	GLenum ConvertTypeToGLType(ColorBufferType type);
	GLenum ConvertOperationToGL(FrameBufferOperationType type);
}