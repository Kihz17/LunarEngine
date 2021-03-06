#include "CascadedShadowMapping.h"
#include "Window.h"
#include "FrameBuffer.h"
#include "TextureArray.h"
#include "TextureManager.h"
#include "ShaderLibrary.h"
#include "Animation.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "InputManager.h"

const std::string CascadedShadowMapping::DEPTH_MAPPING_SHADER_KEY = "shadowMappingDepthShader";
const std::string CascadedShadowMapping::DEPTH_MAPPING_ANIMATED_SHADER_KEY = "animatedShadowMappingDepthShader";
const std::string CascadedShadowMapping::DEPTH_DEBUG_SHADER_KEY = "debugDepthShader";
const int CascadedShadowMapping::MAX_CASCADE_LEVELS = 16;

constexpr unsigned int depthMapResolution = 6144;
constexpr float lightMapColorBorder[] = { 1.0f, 1.0f, 1.0f, 1.0f };

CascadedShadowMapping::CascadedShadowMapping(const CascadedShadowMappingInfo& info)
	: lightDepthBuffer(new FrameBuffer()),
	lightMatricesUBO(new UniformBuffer(sizeof(glm::mat4x4)* MAX_CASCADE_LEVELS, GL_STATIC_DRAW, 0)),
	lightDepthMaps(nullptr),
	softnessTextures(nullptr),
	depthMappingShader(ShaderLibrary::Load(DEPTH_MAPPING_SHADER_KEY, "assets/shaders/CSMDepth.glsl")),
	depthMappingAnimatedShader(ShaderLibrary::Load(DEPTH_MAPPING_ANIMATED_SHADER_KEY, "assets/shaders/CSMDepthAnimated.glsl")),
	cameraFOV(info.cameraFOV),
	cameraView(info.cameraView),
	windowSpecs(info.windowSpecs),
	projectionNearPlane(info.projectionNearPlane),
	projectionFarPlane(info.projectionFarPlane),
	zMult(info.zMult)
{
	// Setup shadow cascade levels
	cascadeLevels.push_back(projectionFarPlane / 40.0f);
	cascadeLevels.push_back(projectionFarPlane / 20.0f);
	cascadeLevels.push_back(projectionFarPlane / 10.0f);
	cascadeLevels.push_back(projectionFarPlane / 5.0f);
	cascadeLevels.push_back(projectionFarPlane / 2.0f);

	// Setup the 3D texture. This is an array of textures that will store a shadow map for each cascade
	// Has to be initialized AFTER our cascade levels are setup
	lightDepthMaps = TextureManager::CreateTextureArray(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, depthMapResolution, depthMapResolution,
		int(cascadeLevels.size()) + 1, TextureFilterType::Nearest, TextureWrapType::ClampToBorder, false);
	lightDepthMaps->TextureParameterFloatArray(GL_TEXTURE_BORDER_COLOR, lightMapColorBorder);

	softnessTextures = TextureManager::CreateTextureArray(GL_RGBA16F, GL_RGBA, GL_FLOAT, depthMapResolution, depthMapResolution,
		int(cascadeLevels.size()) + 1, TextureFilterType::Linear, TextureWrapType::Repeat, true);

	lightDepthBuffer->Bind();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, softnessTextures->GetID(), 0);
	lightDepthBuffer->SetDepthAttachment(lightDepthMaps); // Assign the depth map texture array to our depth buffer fbo

	// Strictly using depth map only, don't waste time writing and reading to color buffer
	//lightDepthBuffer->SetColorBufferWrite(ColorBufferType::None);
	//lightDepthBuffer->SetColorBufferRead(ColorBufferType::None);
	if (!lightDepthBuffer->CheckComplete()) std::cout << "Light Depth Buffer not complete!\n";
	lightDepthBuffer->Unbind();

	// Setup shader uniforms
	depthMappingShader->InitializeUniform("uMatModel");
	depthMappingShader->InitializeUniform("uShadowSoftness");

	// Setup animated shader uniforms
	depthMappingAnimatedShader->InitializeUniform("uMatModel");
	depthMappingAnimatedShader->InitializeUniform("uShadowSoftness");
	for (unsigned int i = 0; i < Animation::MAX_BONES; i++)
	{
		depthMappingAnimatedShader->InitializeUniform("uBoneMatrices[" + std::to_string(i) + "]");
	}
}

CascadedShadowMapping::~CascadedShadowMapping()
{
	delete lightDepthBuffer;
	delete lightMatricesUBO;
}

void CascadedShadowMapping::DoPass(std::vector<RenderSubmission>& submissions, std::vector<RenderSubmission>& animatedSubmissions, const glm::vec3& lightDir, const glm::mat4& projection, const glm::mat4& view, PrimitiveShape& quad)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // Fixes peter panning (shadow offsets)

	// Update the data in our light matrices UBO
	lightMatricesUBO->Bind();
	std::vector<glm::mat4> lightMatrices = GetLightSpaceMatrices(lightDir);
	for (size_t i = 0; i < lightMatrices.size(); i++)
	{
		lightMatricesUBO->SubData(i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	lightMatricesUBO->Unbind();


	lightDepthBuffer->Bind();
	depthMappingShader->Bind();

	// Attach texture array to FBO
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, softnessTextures->GetID(), 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, depthMapResolution, depthMapResolution); // Set viewport to size of the depth map

	for (RenderSubmission& submission : submissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;

		depthMappingShader->SetFloat("uShadowSoftness", submission.renderComponent->castingShadownSoftness);

		renderComponent->Draw(depthMappingShader, submission.transform);
	}

	depthMappingAnimatedShader->Bind();
	for (RenderSubmission& submission : animatedSubmissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;

		for (unsigned int i = 0; i < submission.boneMatricesLength; i++) // Pass bone matrices to shader
		{
			glm::mat4& matrix = submission.boneMatrices[i];
			depthMappingAnimatedShader->SetMat4("uBoneMatrices[" + std::to_string(i) + "]", matrix);
		}

		depthMappingShader->SetFloat("uShadowSoftness", submission.renderComponent->castingShadownSoftness);

		renderComponent->Draw(depthMappingAnimatedShader, submission.transform);
	}

	lightDepthBuffer->Unbind();

	glViewport(0, 0, windowSpecs->width, windowSpecs->height); 	// Set viewport back to source
}

std::vector<glm::vec4> CascadedShadowMapping::GetFrustumCornersWorldSpace(const glm::mat4& projection)
{
	glm::mat4 inverse = glm::inverse(projection * cameraView);

	std::vector<glm::vec4> frustumCorners;

	// Create bounding boxes for the near, middle and far plane of our frustum https://ogldev.org/www/tutorial49/img7.png
	for (unsigned int x = 0; x < 2; x++)
	{
		for (unsigned int y = 0; y < 2; y++)
		{
			for (unsigned int z = 0; z < 2; z++)
			{
				// Apply the inverse of the projection view matrix to the corner points of the NDC (normalized device coordinates -1.0 to 1.0) cube
				// This will result in a frustum corner in world space
				glm::vec4 point = inverse * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(point / point.w);
			}
		}
	}

	return frustumCorners;
}

glm::mat4 CascadedShadowMapping::GetLightSpaceMatrix(const glm::vec3& lightDir, const float nearPlane, const float farPlane)
{
	glm::mat4 projection = glm::perspective(cameraFOV, (float)windowSpecs->width / (float)windowSpecs->height, nearPlane, farPlane);
	std::vector<glm::vec4> frustumCorners = GetFrustumCornersWorldSpace(projection);

	// Get the center of the frustum, this is important because we know for sure that our light source is looking here, so we can construct our light view from this point
	glm::vec3 center(0.0f);
	for (glm::vec4& point : frustumCorners)
	{
		center += glm::vec3(point);
	}
	center /= frustumCorners.size(); // Average corners

	glm::vec3 eye = center + -lightDir * (farPlane - nearPlane);
	glm::mat4 lightView = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));

	// Generate an AABB that fits tightly onto the frustum. This will be used to construct our orthographic projection matrx later on
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::min();
	for (glm::vec4& point : frustumCorners)
	{
		const glm::vec4 viewSpaceCoords = lightView * point; // Obtain the coordinate of a frustum corner in the light's view space
		minX = std::min(minX, viewSpaceCoords.x);
		maxX = std::max(maxX, viewSpaceCoords.x);
		minY = std::min(minY, viewSpaceCoords.y);
		maxY = std::max(maxY, viewSpaceCoords.y);
		minZ = std::min(minZ, viewSpaceCoords.z);
		maxZ = std::max(maxZ, viewSpaceCoords.z);
	}

	// Increase the size of the space covered by our near and far plane
	// We do this so that geometry outside of the frustum can still cast shadows on surfaces inside if the frustum
	{
		// Pull back the near plane
		if (minZ < 0.0f)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}

		// Push away the far plane
		if (maxZ < 0.0f)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ - minZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> CascadedShadowMapping::GetLightSpaceMatrices(const glm::vec3& lightDir)
{
	std::vector<glm::mat4> matrices;
	for (size_t i = 0; i < cascadeLevels.size() + 1; i++)
	{
		if (i == 0) // Get matrix for near plane
		{
			matrices.push_back(GetLightSpaceMatrix(lightDir, projectionNearPlane, cascadeLevels[i]));
		}
		else if (i < cascadeLevels.size()) // Get matrix for middle plane
		{
			matrices.push_back(GetLightSpaceMatrix(lightDir, cascadeLevels[i - 1], cascadeLevels[i]));
		}
		else // Get matrix for far plane
		{
			matrices.push_back(GetLightSpaceMatrix(lightDir, cascadeLevels[i - 1], projectionFarPlane));
		}
	}

	return matrices;
}

// Dir = 0.1, 0.0, 0.9
// Start Pos = 1, 2, 5
// Result = 1.1, 2, 5.9
