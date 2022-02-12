#include "ForwardRenderPass.h"
#include "ShaderLibrary.h"
#include "Light.h"
#include "Renderer.h"

#include <sstream>

ForwardRenderPass::ForwardRenderPass(IFrameBuffer* geometryBuffer)
	: geometryBuffer(geometryBuffer),
	shader(ShaderLibrary::Load(Renderer::FORWARD_SHADER_KEY, "assets/shaders/forward.glsl"))
{
	// Setup shader uniforms
	shader->InitializeUniform("uMatModel");
	shader->InitializeUniform("uMatView");
	shader->InitializeUniform("uMatProjection");
	shader->InitializeUniform("uMatModelInverseTranspose");
	shader->InitializeUniform("uColorOverride");
	shader->InitializeUniform("uAlbedoTexture1");
	shader->InitializeUniform("uAlbedoTexture2");
	shader->InitializeUniform("uAlbedoTexture3");
	shader->InitializeUniform("uAlbedoTexture4");
	shader->InitializeUniform("uAlbedoRatios");
	shader->InitializeUniform("uHasNormalTexture");
	shader->InitializeUniform("uNormalTexture");
	shader->InitializeUniform("uRoughnessTexture");
	shader->InitializeUniform("uMetalnessTexture");
	shader->InitializeUniform("uAmbientOcculsionTexture");
	shader->InitializeUniform("uMaterialOverrides");
	shader->InitializeUniform("uAlphaTransparency");
	shader->InitializeUniform("uIgnoreLighting");

	shader->SetInt("uAlbedoTexture1", 0);
	shader->SetInt("uAlbedoTexture2", 1);
	shader->SetInt("uAlbedoTexture3", 2);
	shader->SetInt("uAlbedoTexture4", 3);
	shader->SetInt("uNormalTexture", 4);
	shader->SetInt("uRoughnessTexture", 5);
	shader->SetInt("uMetalnessTexture", 6);
	shader->SetInt("uAmbientOcculsionTexture", 7);
	shader->InitializeUniform("uLightAmount");

	for (int i = 0; i < Light::MAX_LIGHTS; i++)
	{
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].position";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].direction";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].color";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].param1";
			shader->InitializeUniform(ss.str());
		}
	}
}

ForwardRenderPass::~ForwardRenderPass()
{

}

void ForwardRenderPass::DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const WindowSpecs* windowSpecs)
{
	geometryBuffer->BindRead(); // Bind for read only
	geometryBuffer->UnbindWrite();

	// Copy depth information from the geometry buffer -> default framebuffer
	glBlitFramebuffer(0, 0, windowSpecs->width, windowSpecs->height, 0, 0, windowSpecs->width, windowSpecs->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	geometryBuffer->Unbind();

	shader->Bind();

	// Pass camera related data
	shader->SetMat4("uMatView", view);
	shader->SetMat4("uMatProjection", projection);

	// Draw geometry
	for (RenderSubmission& submission : submissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;

		shader->SetMat4("uMatModel", submission.transform);
		shader->SetMat4("uMatModelInverseTranspose", glm::inverse(submission.transform));

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

		// Normal
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

		// Materials
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

		shader->SetInt("uIgnoreLighting", renderComponent->isIgnoreLighting ? GL_TRUE : GL_FALSE);

		shader->SetFloat("uAlphaTransparency", renderComponent->alphaTransparency);

		if (renderComponent->isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		renderComponent->mesh->GetVertexArray()->Bind();
		glDrawElements(GL_TRIANGLES, renderComponent->mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
		renderComponent->mesh->GetVertexArray()->Unbind();
	}
}