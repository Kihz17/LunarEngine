#pragma once

#include "Window.h"
#include "IFrameBuffer.h"
#include "UniformBuffer.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <vector>

struct CSMShadowMappingInfo
{
	glm::vec3 lightDirection;
	Camera& camera;
	glm::mat4& cameraView;
	WindowSpecs* windowSpecs;
	float& projectionNearPlane;
	float& projectionFarPlane;
	float zMult = 10.0f;
};

class CSMShadowMapping
{
public:
	CSMShadowMapping(const CSMShadowMappingInfo& info);
	virtual ~CSMShadowMapping();

	void Update(float deltaTime);

	void UpdateLightDirection(const glm::vec3& lightDirection) { this->lightDirection = lightDirection; }

private:
	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj);
	glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane);
	std::vector<glm::mat4> GetLightSpaceMatrices();

	IFrameBuffer* lightDepthBuffer;
	std::vector<float> cascadeLevels;
	UniformBuffer* lightMatricesUBO;

	glm::vec3 lightDirection;

	// Pulled from Renderer.h, Renderer will always outlast this class so it's okay to hold references to these objects
	Camera& camera;
	glm::mat4& cameraView;
	WindowSpecs* windowSpecs;
	float& projectionNearPlane;
	float& projectionFarPlane;

	float zMult;
};