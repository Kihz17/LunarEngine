#pragma once

#include "Window.h"
#include "ITexture.h"
#include "Shader.h"
#include "IFrameBuffer.h"
#include "PrimitiveShape.h"

#include <glm/glm.hpp>

#include <string>

class CloudPass
{
public:
	CloudPass(const WindowSpecs* windowSpecs);
	~CloudPass();

	void DoPass(ITexture* positionBuffer, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& cameraDir, PrimitiveShape* quad, const WindowSpecs* windowSpecs);

	ITexture* GetCloudTexture() { return cloudAttachment; }

	static const std::string CLOUD_SHADER_KEY;

	ITexture* shapeNoise;
	ITexture* detailNoise;
	ITexture* offsetTexture;

private:
	IFrameBuffer* frameBuffer;
	Shader* shader;

	ITexture* cloudAttachment;
};