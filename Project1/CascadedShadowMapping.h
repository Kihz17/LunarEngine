#pragma once

#include "RenderSubmission.h"
#include "Window.h"
#include "IFrameBuffer.h"
#include "UniformBuffer.h"
#include "Camera.h"
#include "TextureArray.h"
#include "Shader.h"
#include "PrimitiveShape.h"
#include "Light.h"
#include "Key.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct CascadedShadowMappingInfo
{
public:

	CascadedShadowMappingInfo(glm::mat4& cameraView, const float& fov, float& near, float& far)
		: cameraView(cameraView),
		cameraFOV(fov),
		projectionNearPlane(near),
		projectionFarPlane(far),
		windowSpecs(nullptr),
		zMult(10.0f)
	{}

	const float& cameraFOV;
	glm::mat4& cameraView;
	const WindowSpecs* windowSpecs;
	const float& projectionNearPlane;
	const float& projectionFarPlane;
	float zMult;
};

class CascadedShadowMapping
{
public:
	CascadedShadowMapping(const CascadedShadowMappingInfo& info);
	virtual ~CascadedShadowMapping();

	void DoPass(std::vector<RenderSubmission*>& submissions, std::vector<RenderSubmission*>& animatedSubmissions, const glm::vec3& lightDir, const glm::mat4& projection, const glm::mat4& view, PrimitiveShape& quad);

	std::vector<float>& GetCascadeLevels() { return cascadeLevels; }
	ITexture* GetShadowMap() { return lightDepthMaps; }
	ITexture* GetSoftnessTexture() { return softnessTextures; }

	static const std::string DEPTH_MAPPING_SHADER_KEY;
	static const std::string DEPTH_MAPPING_ANIMATED_SHADER_KEY;
	static const std::string DEPTH_DEBUG_SHADER_KEY;
	static const int MAX_CASCADE_LEVELS;
private:
	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj);
	glm::mat4 GetLightSpaceMatrix(const glm::vec3& lightDir, const float nearPlane, const float farPlane);
	std::vector<glm::mat4> GetLightSpaceMatrices(const glm::vec3& lightDir);

	IFrameBuffer* lightDepthBuffer;
	std::vector<float> cascadeLevels;
	UniformBuffer* lightMatricesUBO;
	TextureArray* lightDepthMaps;
	TextureArray* softnessTextures;

	Shader* depthMappingShader;
	Shader* depthMappingAnimatedShader;

	// Pulled from Renderer.h, Renderer will always outlast this class so it's okay to hold references to these objects
	glm::mat4& cameraView;
	const WindowSpecs* windowSpecs;
	const float& cameraFOV;
	const float& projectionNearPlane;
	const float& projectionFarPlane;

	float zMult;
};