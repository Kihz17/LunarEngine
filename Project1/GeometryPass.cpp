#include "GeometryPass.h"
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "ShaderLibrary.h"

GeometryPass::GeometryPass(const WindowSpecs* windowSpecs, glm::vec3& cameraPos)
	: geometryBuffer(new FrameBuffer()),
	geometryRenderBuffer(new RenderBuffer(GL_DEPTH_COMPONENT, windowSpecs->width, windowSpecs->height)),
	shader(ShaderLibrary::Load(G_SHADER_KEY, "assets/shaders/geometryBuffer.glsl")),
	windowSpecs(windowSpecs),
	cameraPosition(cameraPos)
{
	// Setup frame buffer color attachments
	geometryBuffer->Bind();
	geometryBuffer->AddColorAttachment2D("position", TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge), 0); // Position Buffer & Depth
	geometryBuffer->AddColorAttachment2D("albedo", TextureManager::CreateTexture2D(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None), 1); // Albedo & Roughness
	geometryBuffer->AddColorAttachment2D("normal", TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None), 2); // Normal & Metalness
	geometryBuffer->AddColorAttachment2D("effects", TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None), 3); // Ambient Occulsion & Velocity

	// Attach a render buffer to our FBO
	geometryBuffer->SetRenderBuffer(geometryRenderBuffer, GL_DEPTH_ATTACHMENT);
	geometryBuffer->Unbind();

	// Initialize geometry shader uniforms
	shader->Bind();
	shader->InitializeUniform("uMatModel");
	shader->InitializeUniform("uMatModelInverseTranspose");
	shader->InitializeUniform("uMatView");
	shader->InitializeUniform("uMatProjection");
	shader->InitializeUniform("uMatProjViewModel");
	shader->InitializeUniform("uMatPrevProjViewModel");
	shader->InitializeUniform("uAlbedoTexture1");
	shader->InitializeUniform("uAlbedoTexture2");
	shader->InitializeUniform("uAlbedoTexture3");
	shader->InitializeUniform("uAlbedoTexture4");
	shader->InitializeUniform("uAlbedoRatios");
	shader->InitializeUniform("uColorOverride");
	shader->InitializeUniform("uHasNormalTexture");
	shader->InitializeUniform("uNormalTexture");
	shader->InitializeUniform("uRoughnessTexture");
	shader->InitializeUniform("uMetalnessTexture");
	shader->InitializeUniform("uAmbientOcculsionTexture");
	shader->InitializeUniform("uMaterialOverrides");
	shader->InitializeUniform("uCanCastShadowOn");
	shader->InitializeUniform("uRRMap");
	shader->InitializeUniform("uRRInfo");
	shader->InitializeUniform("uCameraPosition");
	shader->Unbind();
}

GeometryPass::~GeometryPass()
{
	delete geometryBuffer;
	delete geometryRenderBuffer;
}

void GeometryPass::DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view)
{
	glDisable(GL_BLEND); // No blend for deffered rendering
	glEnable(GL_DEPTH_TEST); // Enable depth testing for scene render

	geometryBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->Bind();

	shader->SetMat4("uMatProjection", projection);
	shader->SetMat4("uMatView", view);
	shader->SetFloat3("uCameraPosition", cameraPosition);

	for (RenderSubmission& submission : submissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;
		glm::mat4 transform = glm::mat4(1.0f);
		transform *= glm::translate(glm::mat4(1.0f), submission.position);
		transform *= glm::toMat4(submission.rotation);
		transform *= glm::scale(glm::mat4(1.0f), submission.scale);

		glm::mat4 projViewModel = projection * view * transform;
		glm::mat4& prevProjViewModel = renderComponent->hasPrevProjViewModel ? renderComponent->projViewModel : projViewModel;
		renderComponent->projViewModel = projViewModel;

		shader->SetMat4("uMatModel", transform);
		shader->SetMat4("uMatModelInverseTranspose", glm::inverse(transform));
		shader->SetMat4("uMatProjViewModel", projViewModel);
		shader->SetMat4("uMatPrevProjViewModel", prevProjViewModel);
		shader->SetInt("uCanCastShadowOn", (GLboolean) renderComponent->castShadowsOn);

		// Color
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

		if (renderComponent->HasMaterialTextures())
		{
			shader->SetFloat4("uMaterialOverrides", glm::vec4(0.0f));

			renderComponent->roughnessTexture->BindToSlot(5);
			shader->SetInt("uRoughnessTexture", 5);

			renderComponent->metalTexture->BindToSlot(6);
			shader->SetInt("uMetalnessTexture", 6);

			renderComponent->aoTexture->BindToSlot(7);
			shader->SetInt("uAmbientOcculsionTexture", 7);
		}
		else // We have no material textures
		{
			shader->SetFloat4("uMaterialOverrides", glm::vec4(renderComponent->roughness, renderComponent->metalness, renderComponent->ao, 1.0f));
		}

		float rrType = renderComponent->reflectRefractType == RRType::Reflect ? 1.0f : renderComponent->reflectRefractType == RRType::Refract ? 2.0f : 0.0f;
		shader->SetFloat4("uRRInfo", glm::vec4(rrType, renderComponent->reflectRefractStrength, renderComponent->refractRatio, 0.0f));

		if (renderComponent->reflectRefractType != RRType::None)
		{
			renderComponent->reflectRefractMap->BindToSlot(8);
		}

		shader->SetInt("uRRMap", 8); // This has to be outside of the if because for some strange reason sampling from a cube map that hasn't been set stops everything from rendering, even if the code isn't run

		if (renderComponent->isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		renderComponent->vao->Bind();
		glDrawElements(GL_TRIANGLES, renderComponent->indexCount, GL_UNSIGNED_INT, 0);
		renderComponent->vao->Unbind();
	}

	// Grass
	//const Shader* grassShader = ShaderLibrary::Get(GRASS_SHADER_KEY);
	//grassShader->Bind();

	//grassVAO->Bind();
	//glDrawArrays(GL_POINTS, 0, 100);

	geometryBuffer->Unbind();
}