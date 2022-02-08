#pragma once

#include "IFrameBuffer.h"

#include <vector>

struct ColorBuffer
{
	std::string name;
	ITexture* texture = nullptr;
};

struct DepthBuffer
{
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
	virtual void AddColorAttachment2D(const std::string& name, ITexture* texture, unsigned int index) override;

	virtual void SetRenderBuffer(IRenderBuffer* renderBuffer, GLenum attachmentType) const override;


	void SetDepthBuffer(ITexture* texture, int level);

private:
	void UpdateAttachmentArray();

	GLuint ID;
	GLuint renderBufferID;

	std::vector<ColorBuffer> colorBuffers;

	DepthBuffer depthBuffer;
	unsigned int* attachments;
};