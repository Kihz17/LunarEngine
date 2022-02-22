#include "DynamicCubeMapRenderer.h"
#include "TextureManager.h"
#include "FrameBuffer.h"
#include "Frustum.h"
#include "ShaderLibrary.h"
#include "Renderer.h"
#include "Utils.h"
#include "EnvironmentMapPass.h"
#include "RenderBuffer.h"

constexpr unsigned int dynamicMapResolution = 512;

DynamicCubeMapRenderer::DynamicCubeMapRenderer(const WindowSpecs* windowSpecs)
	: frameBuffer(new FrameBuffer()),
	cubeMapFrameBuffer(new FrameBuffer()),
	cubeMap(TextureManager::CreateCubeMap(dynamicMapResolution, GL_LINEAR_MIPMAP_LINEAR, GL_RGB, GL_RGB16F, GL_FLOAT)),
	depthBuffer(new RenderBuffer(GL_DEPTH_COMPONENT, dynamicMapResolution, dynamicMapResolution)),
	shader(ShaderLibrary::Load("dynamicCubeMapGShader", "assets/shaders/dynamicCubeMapGeometry.glsl")),
	conversionShader(ShaderLibrary::Load("dynamicCubeMapCShader", "assets/shaders/dynamicCubeMapConverter.glsl")),
	quad(ShapeType::Quad)
{
	frameBuffer->Bind();
	frameBuffer->AddColorAttachment2D("environment", TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, dynamicMapResolution, dynamicMapResolution, TextureFilterType::Linear, TextureWrapType::None), 0);
	frameBuffer->SetRenderBuffer(depthBuffer, GL_DEPTH_ATTACHMENT);
	frameBuffer->Unbind();

	shader->Bind();
	shader->InitializeUniform("uMatModel");
	shader->InitializeUniform("uMatView");
	shader->InitializeUniform("uMatProjection");
	shader->InitializeUniform("uAlbedoTexture1");
	shader->InitializeUniform("uAlbedoTexture2");
	shader->InitializeUniform("uAlbedoTexture3");
	shader->InitializeUniform("uAlbedoTexture4");
	shader->InitializeUniform("uAlbedoRatios");
	shader->InitializeUniform("uColorOverride");
	shader->InitializeUniform("uHasNormalTexture");
	shader->InitializeUniform("uNormalTexture");
	shader->Unbind();

	conversionShader->Bind();
	conversionShader->InitializeUniform("uFaceBuffer");
	conversionShader->InitializeUniform("uEnvMap");
	conversionShader->InitializeUniform("uCubeFace");
	conversionShader->Unbind();
}

DynamicCubeMapRenderer::~DynamicCubeMapRenderer()
{
	delete frameBuffer;
	delete cubeMapFrameBuffer;
	delete depthBuffer;
	TextureManager::DeleteTexture(cubeMap);
}

CubeMap* DynamicCubeMapRenderer::GenerateDynamicCubeMap(const glm::vec3& center, std::unordered_map<int, std::vector<RenderSubmission*>>& allSubmissions, 
	const float fov, const float far, const float near, const WindowSpecs* windowSpecs)
{
	constexpr glm::vec3 directions[] = { 
		glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) };

	constexpr glm::vec3 upVectors[] = {
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

	glViewport(0, 0, dynamicMapResolution, dynamicMapResolution); // Make sure we configure the viewport to capture the texture dimensions

	glm::mat4 projection = glm::perspective(fov, (float)dynamicMapResolution / (float)dynamicMapResolution, near, far);

	for (int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++)
	{
		std::vector<RenderSubmission*> submissions = allSubmissions[i];

		int cubeMapIndex = i - (int)GL_TEXTURE_CUBE_MAP_POSITIVE_X;

		glm::vec3 direction = directions[cubeMapIndex];
		glm::vec3 upVec = upVectors[cubeMapIndex];
		glm::mat4 view = glm::lookAt(center, center + direction, upVec);

		frameBuffer->Bind();

		glEnable(GL_DEPTH_TEST); // Enable depth test when drawing scene
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw geometry
		shader->Bind();
		shader->SetMat4("uMatView", view);
		shader->SetMat4("uMatProjection", projection);

		frameBuffer->ClearColorBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Make sure we clear the color buffer with 1.0 at index w incase there is no geometry

		for (RenderSubmission* submissions : submissions)
		{
			Submesh* renderComponent = submissions->submesh;

			shader->SetMat4("uMatModel", submissions->transform);

			// Diffuse color
			if (renderComponent->isColorOverride)
			{
				shader->SetFloat4("uColorOverride", glm::vec4(renderComponent->colorOverride.x, renderComponent->colorOverride.y, renderComponent->colorOverride.z, 1.0f));
			}
			else // Bind diffuse textures
			{
				shader->SetFloat4("uColorOverride", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

				float ratios[4];
				for (int i = 0; i < 4; i++)
				{
					if (i < renderComponent->albedoTextures.size())
					{
						renderComponent->albedoTextures[i].first->BindToSlot(i);
						shader->SetInt(std::string("uAlbedoTexture" + std::to_string(i + 1)), i);
						ratios[i] = renderComponent->albedoTextures[i].second;
					}
					else
					{
						ratios[i] = 0.0f;
					}
				}
				shader->SetFloat4("uAlbedoRatios", glm::vec4(ratios[0], ratios[1], ratios[2], ratios[3]));
			}

			// Normals
			if (renderComponent->normalTexture)
			{
				shader->SetInt("uHasNormalTexture", GL_TRUE);
				renderComponent->normalTexture->BindToSlot(4);
				shader->SetInt("uNormalTexture", 4);
			}
			else
			{
				shader->SetInt("uHasNormalTexture", GL_FALSE);
			}

			renderComponent->mesh->GetVertexArray()->Bind();
			glDrawElements(GL_TRIANGLES, renderComponent->mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
			renderComponent->mesh->GetVertexArray()->Unbind();
		}

		frameBuffer->Unbind();

		// Map the scene we just drew to a face of the cube map
		cubeMapFrameBuffer->Bind();
		conversionShader->Bind();

		glDisable(GL_DEPTH_TEST); // Disable depth test when cube map face
		glDisable(GL_CULL_FACE);

		cubeMapFrameBuffer->AddColorAttachmentCubeMapFace("cubeFace", cubeMap, 0, (CubeMapFace)cubeMapIndex);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::GetEnvironmentMapCube()->BindToSlot(0);
		conversionShader->SetInt("uEnvMap", 0);

		frameBuffer->GetColorAttachment("environment")->BindToSlot(1);
		conversionShader->SetInt("uFaceBuffer", 1);

		conversionShader->SetInt("uCubeFace", cubeMapIndex); // Tell shader what face we are mapping to

		quad.Draw();

		cubeMapFrameBuffer->Unbind();
	}

	cubeMap->ComputeMipmap();

	return cubeMap;
}