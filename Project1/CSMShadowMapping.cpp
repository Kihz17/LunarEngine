#include "CSMShadowMapping.h"
#include "Window.h"
#include "FrameBuffer.h"
#include "TextureArray.h"
#include "TextureManager.h"

#include <glm/gtc/matrix_transform.hpp>

constexpr unsigned int maxShadowMaps = 16;
constexpr unsigned int depthMapResolution = 4096;
constexpr float lightMapColorBorder[] = { 1.0f, 1.0f, 1.0f, 1.0f };

CSMShadowMapping::CSMShadowMapping(const CSMShadowMappingInfo& info)
	: lightDepthBuffer(new FrameBuffer()),
	lightMatricesUBO(new UniformBuffer(sizeof(glm::mat4x4) * maxShadowMaps, GL_STATIC_DRAW, 0)),
	lightDirection(info.lightDirection),
	camera(info.camera),
	cameraView(info.cameraView),
	windowSpecs(info.windowSpecs),
	projectionNearPlane(info.projectionNearPlane),
	projectionFarPlane(info.projectionFarPlane),
	zMult(info.zMult)
{
	lightDepthBuffer->Bind();

	// Setup shadow cascade levels
	cascadeLevels.push_back(projectionFarPlane / 50.0f);
	cascadeLevels.push_back(projectionFarPlane / 25.0f);
	cascadeLevels.push_back(projectionFarPlane / 10.0f);
	cascadeLevels.push_back(projectionFarPlane / 2.0f);

	// Setup the 3D texture. This is an array of textures that will store a shadow map for each cascade
	TextureArray* lightDepthMaps = TextureManager::CreateTextureArray(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, depthMapResolution, depthMapResolution,
		int(cascadeLevels.size()) + 1, TextureFilterType::Nearest, TextureWrapType::ClampToBorder, false);
	lightDepthMaps->TextureParameterFloatArray(GL_TEXTURE_BORDER_COLOR, lightMapColorBorder);

	//lightDepthBuffer->SetDepthBuffer(lightDepthMaps, 0); // Assign the depth map texture array to our depth buffer fbo

	// Strictly using depth map only, don't waste time writing and reading to color buffer
	lightDepthBuffer->SetColorBufferWrite(ColorBufferType::None);
	lightDepthBuffer->SetColorBufferRead(ColorBufferType::None);
	lightDepthBuffer->Unbind();
}

CSMShadowMapping::~CSMShadowMapping()
{
	delete lightDepthBuffer;
	delete lightMatricesUBO;
}

void CSMShadowMapping::Update(float deltaTime)
{
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
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, );
	lightDepthBuffer->Unbind();
}

std::vector<glm::vec4> CSMShadowMapping::GetFrustumCornersWorldSpace(const glm::mat4& projection)
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

glm::mat4 CSMShadowMapping::GetLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	glm::mat4 projection = glm::perspective(camera.fov, (float) windowSpecs->width / (float) windowSpecs->height, nearPlane, farPlane);
	std::vector<glm::vec4> frustumCorners = GetFrustumCornersWorldSpace(projection);

	// Get the center of the frustum, this is important because we know for sure that our light source is looking here, so we can construct our light view from this point
	glm::vec3 center(0.0f);
	for (glm::vec4& point : frustumCorners)
	{
		center += glm::vec3(point);
	}
	center /= frustumCorners.size();

	glm::mat4 lightView = glm::lookAt(center + lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));

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

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> CSMShadowMapping::GetLightSpaceMatrices()
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