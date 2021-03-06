#pragma once

#include "Window.h"
#include "Texture3D.h"
#include "Shader.h"
#include "IFrameBuffer.h"
#include "PrimitiveShape.h"
#include "Light.h"
#include "ComputeShader.h"

#include <glm/glm.hpp>

#include <string>

class CloudPass
{
public:
	CloudPass(float earthRadius, float innerRadius, float outerRadius, float cutoffFactor, const WindowSpecs* windowSpecs);
	~CloudPass();

	void DoPass(ITexture* skyTexture, ITexture* positionBuffer, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& cameraDir,
		const WindowSpecs* windowSpecs, PrimitiveShape* quad);

	Texture2D* GetSkyTexture() { return postColorAttachment; }

	static const std::string CLOUD_SHADER_KEY;
	static const std::string WEATHER_SHADER_KEY;
	static const std::string PERLIN_WORLEY_SHADER_KEY;
	static const std::string WORLEY_SHADER_KEY;
	static const std::string CLOUD_POST_SHADER_KEY;

	Texture3D* cloudTexture;
	Texture3D* worleyTexture;
	Texture2D* weatherTexture;

	glm::vec3 weatherSeed;
	float weatherPerlinFreq;

	bool enableGodRays;

	float coverage;
	float cloudSpeed;
	float crispiness;
	float detail;
	float density;
	float absorptionToLight;
	float cloudDarkness;
	float cloudCutoffFactor;

	float earthRadius;
	float sphereInnerRadius;
	float sphereOuterRadius;

	float godRayDecay;
	float godRayDensity;
	float godRayWeight;
	float godRayExposure;

	float perlinFreq;

	glm::vec3 cloudColorTop;
	glm::vec3 cloudColorBottom;

private:
	void GenerateTextures(const glm::ivec3& perlinWorleyDimensions, const glm::ivec3& worleyDimensions, const glm::ivec2& weatherDimensions);

	ComputeShader* cloudShader;
	ComputeShader* weatherShader;
	ComputeShader* perlinWorleyShader;
	ComputeShader* worleyShader;

	// Written to by compute shader
	Texture2D* colorTexture;
	Texture2D* bloomTexture;

	Shader* postShader;
	IFrameBuffer* postFramebuffer;
	Texture2D* postDepthAttachment;
	Texture2D* postColorAttachment;
};