#pragma once

#include "IFrameBuffer.h"

#include <vector>

struct ColorBuffer
{
	std::string name;
	ITexture* texture = nullptr;
};

class FrameBuffer : public IFrameBuffer
{
public:
	FrameBuffer();
	virtual ~FrameBuffer();

	virtual void Bind() const override;
	virtual void BindRead() const override;
	virtual void BindWrite() const override;

	virtual void Unbind() const override;
	virtual void UnbindRead() const override;;
	virtual void UnbindWrite() const override;

	virtual void SetColorBufferWrite(ColorBufferType type) const override;
	virtual void SetColorBufferWriteAttachment(unsigned int attachment) const override;

	virtual void SetColorBufferRead(ColorBufferType type) const override;
	virtual void SetColorBufferReadAttachment(unsigned int attachment) const override;

	virtual bool CheckComplete() const override;

	virtual ITexture* GetColorAttachment(const std::string& name) override;
	virtual void AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite) override;
	virtual void AddColorAttachmentCubeMapFace(const std::string& name, CubeMap* texture, unsigned int index, CubeMapFace face, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite) override;

	virtual void SetDepthAttachment(ITexture* texture, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite, int level = 0) override;
	virtual void SetDepthAttachment2D(Texture2D* texture, FrameBufferOperationType operationType = FrameBufferOperationType::ReadWrite, int level = 0) override;

	virtual void SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const override;

	virtual void ClearColorBuffer(const glm::vec4& value, unsigned int index = 0) const override;

private:
	void UpdateAttachmentArray();

	GLuint ID;
	GLuint renderBufferID;

	std::vector<ColorBuffer> colorBuffers;

	ITexture* depthBuffer;

	unsigned int* attachments;
};