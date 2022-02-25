#include "CascadedShadowMapping.h"
#include "Window.h"
#include "FrameBuffer.h"
#include "TextureArray.h"
#include "TextureManager.h"
#include "ShaderLibrary.h"
#include "Animation.h"

#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "InputManager.h"

const std::string CascadedShadowMapping::DEPTH_MAPPING_SHADER_KEY = "shadowMappingDepthShader";
const std::string CascadedShadowMapping::DEPTH_MAPPING_ANIMATED_SHADER_KEY = "animatedShadowMappingDepthShader";
const std::string CascadedShadowMapping::DEPTH_DEBUG_SHADER_KEY = "debugDepthShader";
const int CascadedShadowMapping::MAX_CASCADE_LEVELS = 16;

constexpr unsigned int depthMapResolution = 4096;
constexpr float lightMapColorBorder[] = { 1.0f, 1.0f, 1.0f, 1.0f };

int cascadeLayer = 0;

CascadedShadowMapping::CascadedShadowMapping(const CascadedShadowMappingInfo& info)
	: lightDepthBuffer(new FrameBuffer()),
	lightMatricesUBO(new UniformBuffer(sizeof(glm::mat4x4)* MAX_CASCADE_LEVELS, GL_STATIC_DRAW, 0)),
	lightDepthMaps(nullptr),
	depthMappingShader(ShaderLibrary::Load(DEPTH_MAPPING_SHADER_KEY, "assets/shaders/CSMDepth.glsl")),
	depthMappingAnimatedShader(ShaderLibrary::Load(DEPTH_MAPPING_ANIMATED_SHADER_KEY, "assets/shaders/CSMDepthAnimated.glsl")),
	depthDebugShader(ShaderLibrary::Load(DEPTH_DEBUG_SHADER_KEY, "assets/shaders/CMSDepthDebug.glsl")),
	directionalLight(nullptr),
	cameraFOV(info.cameraFOV),
	cameraView(info.cameraView),
	windowSpecs(info.windowSpecs),
	projectionNearPlane(info.projectionNearPlane),
	projectionFarPlane(info.projectionFarPlane),
	zMult(info.zMult),
	quad(ShapeType::Quad)
{
	// Setup shadow cascade levels
	cascadeLevels.push_back(projectionFarPlane / 40.0f);
	cascadeLevels.push_back(projectionFarPlane / 25.0f);
	cascadeLevels.push_back(projectionFarPlane / 10.0f);
	cascadeLevels.push_back(projectionFarPlane / 5.0f);
	cascadeLevels.push_back(projectionFarPlane / 2.0f);

	// Setup the 3D texture. This is an array of textures that will store a shadow map for each cascade
	// Has to be initialized AFTER our cascade levels are setup
	lightDepthMaps = TextureManager::CreateTextureArray(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, depthMapResolution, depthMapResolution,
		int(cascadeLevels.size()) + 1, TextureFilterType::Nearest, TextureWrapType::ClampToBorder, false);
	lightDepthMaps->TextureParameterFloatArray(GL_TEXTURE_BORDER_COLOR, lightMapColorBorder);

	lightDepthBuffer->Bind();
	lightDepthBuffer->SetDepthAttachment(lightDepthMaps); // Assign the depth map texture array to our depth buffer fbo

	// Strictly using depth map only, don't waste time writing and reading to color buffer
	lightDepthBuffer->SetColorBufferWrite(ColorBufferType::None);
	lightDepthBuffer->SetColorBufferRead(ColorBufferType::None);
	if (!lightDepthBuffer->CheckComplete()) std::cout << "Light Depth Buffer not complete!\n";
	lightDepthBuffer->Unbind();

	// Setup shader uniforms
	depthMappingShader->Bind();
	depthMappingShader->InitializeUniform("uMatModel");
	depthMappingShader->Unbind();

	depthMappingAnimatedShader->Bind();
	depthMappingAnimatedShader->InitializeUniform("uMatModel");
	for (unsigned int i = 0; i < Animation::MAX_BONES; i++)
	{
		depthMappingAnimatedShader->InitializeUniform("uBoneMatrices[" + std::to_string(i) + "]");
	}
	depthMappingAnimatedShader->Unbind();

	// FOR DEBUGGING
	depthDebugShader->Bind();
	depthDebugShader->InitializeUniform("uLayer");
	depthDebugShader->InitializeUniform("uDepthTexture");
	depthDebugShader->Unbind();

	testKey = InputManager::GetKey(GLFW_KEY_L);
}

CascadedShadowMapping::~CascadedShadowMapping()
{
	delete lightDepthBuffer;
	delete lightMatricesUBO;
}

void CascadedShadowMapping::DoPass(std::vector<RenderSubmission*>& submissions, std::vector<RenderSubmission*>& animatedSubmissions, const glm::mat4& projection, const glm::mat4& view)
{
	if (!directionalLight) return; // No directional light, don't map anything

	// Debugging
	/*if (testKey->IsJustPressed())
	{
		cascadeLayer++;
		if (cascadeLayer > cascadeLevels.size())
		{
			cascadeLayer = 0;
		}
	}*/

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the data in our light matrices UBO
	lightMatricesUBO->Bind();
	std::vector<glm::mat4> lightMatrices = GetLightSpaceMatrices();
	for (size_t i = 0; i < lightMatrices.size(); i++)
	{
		lightMatricesUBO->SubData(i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	lightMatricesUBO->Unbind();

	// Generate depth maps by rendering the scene from the light's POV
	lightDepthBuffer->Bind();
	depthMappingShader->Bind();

	glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, lightDepthMaps->GetID(), 0);
	glViewport(0, 0, depthMapResolution, depthMapResolution); // Set viewport to size of the depth map
	glClear(GL_DEPTH_BUFFER_BIT); // Re-clear depth buffer
	glCullFace(GL_FRONT); // Fixes peter panning (shadow offsets)

	for (RenderSubmission* submission : submissions)
	{
		RenderComponent* renderComponent = submission->renderComponent;
		if (!renderComponent->castShadows) continue;

		// TODO: If object is semi-transparent, make a softer shadow
		// To do this, we will need to change the texture array into a color attachment instead of a depth attachment
		// The CMS fragment shader will now need to write the depth to a channel (geometry's z value from the light's perspective), and write
		// the "softness" value to another channel

		// TODO: Another issue will arise with this ^^^. The "softness" value will be directly associated with the surface the shadow is being casted on, NOT the object casting the shadow.
		// This is a problem that will need a solution

		renderComponent->Draw(depthMappingShader, submission->transform);
	}

	depthMappingAnimatedShader->Bind();
	for (RenderSubmission* submission : animatedSubmissions)
	{
		RenderComponent* renderComponent = submission->renderComponent;
		if (!renderComponent->castShadows) continue;

		for (unsigned int i = 0; i < submission->boneMatricesLength; i++) // Pass bone matrices to shader
		{
			glm::mat4& matrix = submission->boneMatrices[i];
			depthMappingAnimatedShader->SetMat4("uBoneMatrices[" + std::to_string(i) + "]", matrix);
		}

		renderComponent->Draw(depthMappingAnimatedShader, submission->transform);
	}

	lightDepthBuffer->Unbind();

	glCullFace(GL_BACK); // Set face culling back to normal
	glViewport(0, 0, windowSpecs->width, windowSpecs->height); 	// Set viewport back to source

	// FOR DEBUGGING
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//depthDebugShader->Bind();
	//depthDebugShader->SetInt("uLayer", cascadeLayer);
	//lightDepthMaps->BindToSlot(0);
	//depthDebugShader->SetInt("uDepthTexture", 0);
	//quad.Draw();
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

glm::mat4 CascadedShadowMapping::GetLightSpaceMatrix(const float nearPlane, const float farPlane)
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

	glm::vec3 eye = center + -directionalLight->GetDirection() * (farPlane - nearPlane);
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

std::vector<glm::mat4> CascadedShadowMapping::GetLightSpaceMatrices()
{
	std::vector<glm::mat4> matrices;
	for (size_t i = 0; i < cascadeLevels.size() + 1; i++)
	{
		if (i == 0) // Get matrix for near plane
		{
			matrices.push_back(GetLightSpaceMatrix(projectionNearPlane, cascadeLevels[i]));
		}
		else if (i < cascadeLevels.size()) // Get matrix for middle plane
		{
			matrices.push_back(GetLightSpaceMatrix(cascadeLevels[i - 1], cascadeLevels[i]));
		}
		else // Get matrix for far plane
		{
			matrices.push_back(GetLightSpaceMatrix(cascadeLevels[i - 1], projectionFarPlane));
		}
	}

	return matrices;
}