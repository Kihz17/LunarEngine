#pragma once

#include "RenderSubmission.h"
#include "IFrameBuffer.h"
#include "IRenderBuffer.h"
#include "Window.h"
#include "Shader.h"
#include "SimpleFastVector.h"

#include <glm/glm.hpp>

#include <string>

class GeometryPass
{
public:
	GeometryPass(const WindowSpecs* windowSpecs);
	virtual ~GeometryPass();

	void DoPass(std::vector<RenderSubmission>& submissions, std::vector<RenderSubmission>& animatedSubmissions, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition);

	IFrameBuffer* GetGBuffer() { return geometryBuffer; }

	ITexture* GetPositionBuffer() { return positionBuffer; }
	ITexture* GetAlbedoBuffer() { return albedoBuffer; }
	ITexture* GetNormalBuffer() { return normalBuffer; }
	ITexture* GetEffectsBuffer() { return effectsBuffer; }

	static const std::string G_SHADER_KEY;
	static const std::string ANIM_SHADER_KEY;

private:
	void PassSharedData(Shader* shader, RenderSubmission& submission, const glm::mat4& projection, const glm::mat4& view);

	IFrameBuffer* geometryBuffer;
	IRenderBuffer* geometryRenderBuffer;

	ITexture* positionBuffer;
	ITexture* albedoBuffer;
	ITexture* normalBuffer;
	ITexture* effectsBuffer;

	Shader* shader;
	Shader* animatedShader;

	const WindowSpecs* windowSpecs;
};