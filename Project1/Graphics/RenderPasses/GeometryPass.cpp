#include "GeometryPass.h"
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "ShaderLibrary.h"
#include "Renderer.h"
#include "Animation.h"

const std::string GeometryPass::G_SHADER_KEY = "gShader";
const std::string GeometryPass::ANIM_SHADER_KEY = "animShader";

GeometryPass::GeometryPass(const WindowSpecs* windowSpecs)
	: geometryBuffer(new FrameBuffer()),
	geometryRenderBuffer(new RenderBuffer(GL_DEPTH_COMPONENT, windowSpecs->width, windowSpecs->height)),
	positionBuffer(TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge)),
	albedoBuffer(TextureManager::CreateTexture2D(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None)),
	normalBuffer(TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None)),
	effectsBuffer(TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None)),
	shader(ShaderLibrary::Load(G_SHADER_KEY, "assets/shaders/geometryBuffer.glsl")),
	animatedShader(ShaderLibrary::Load(ANIM_SHADER_KEY, "assets/shaders/animatedGeometryBuffer.glsl")),
	windowSpecs(windowSpecs)
{
	// Setup frame buffer color attachments
	geometryBuffer->Bind();
	geometryBuffer->AddColorAttachment2D("position", positionBuffer, 0); // Position Buffer & Depth
	geometryBuffer->AddColorAttachment2D("albedo", albedoBuffer, 1); // Albedo & Roughness
	geometryBuffer->AddColorAttachment2D("normal", normalBuffer, 2); // Normal & Metalness
	geometryBuffer->AddColorAttachment2D("effects", effectsBuffer, 3); // Ambient Occulsion & Velocity
	//geometryBuffer->SetDepthAttachment(TextureManager::CreateTexture2D(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_FLOAT,
	//	windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::ClampToBorder));

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
	shader->InitializeUniform("uORMTexture");
	shader->InitializeUniform("uMaterialOverrides");
	shader->InitializeUniform("uShadowSoftness");
	shader->InitializeUniform("uRRMap");
	shader->InitializeUniform("uRRInfo");
	shader->InitializeUniform("uCameraPosition");
	shader->InitializeUniform("uUVOffset");
	shader->Unbind();

	// Initialize animated shader uniforms
	animatedShader->Bind();
	animatedShader->InitializeUniform("uMatModel");
	animatedShader->InitializeUniform("uMatModelInverseTranspose");
	animatedShader->InitializeUniform("uMatView");
	animatedShader->InitializeUniform("uMatProjection");
	animatedShader->InitializeUniform("uMatProjViewModel");
	animatedShader->InitializeUniform("uMatPrevProjViewModel");
	animatedShader->InitializeUniform("uAlbedoTexture1");
	animatedShader->InitializeUniform("uAlbedoTexture2");
	animatedShader->InitializeUniform("uAlbedoTexture3");
	animatedShader->InitializeUniform("uAlbedoTexture4");
	animatedShader->InitializeUniform("uAlbedoRatios");
	animatedShader->InitializeUniform("uColorOverride");
	animatedShader->InitializeUniform("uHasNormalTexture");
	animatedShader->InitializeUniform("uNormalTexture");
	animatedShader->InitializeUniform("uORMTexture");
	animatedShader->InitializeUniform("uMaterialOverrides");
	animatedShader->InitializeUniform("uShadowSoftness");
	animatedShader->InitializeUniform("uRRMap");
	animatedShader->InitializeUniform("uRRInfo");
	animatedShader->InitializeUniform("uCameraPosition");
	animatedShader->InitializeUniform("uUVOffset");
	for (unsigned int i = 0; i < Animation::MAX_BONES; i++)
	{
		animatedShader->InitializeUniform("uBoneMatrices[" + std::to_string(i) + "]");
	}
	animatedShader->Unbind();
}

GeometryPass::~GeometryPass()
{
	delete geometryBuffer;
	delete geometryRenderBuffer;
}

void GeometryPass::DoPass(std::vector<RenderSubmission>& submissions, std::vector<RenderSubmission>& animatedSubmissions, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition)
{
	glDisable(GL_BLEND); // No blend for deffered rendering
	glEnable(GL_DEPTH_TEST); // Enable depth testing for scene render
	glEnable(GL_MULTISAMPLE); // AA

	geometryBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->Bind();

	shader->SetMat4("uMatProjection", projection);
	shader->SetMat4("uMatView", view);
	shader->SetFloat3("uCameraPosition", cameraPosition);

	// Draw static meshes
	for (RenderSubmission& submission : submissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;

		if (renderComponent->faceCullType == FaceCullType::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(renderComponent->faceCullType == FaceCullType::Front ? GL_FRONT : GL_BACK);
		}
		
		PassSharedData(shader, submission, projection, view);

		renderComponent->Draw(shader, submission.transform);
	}

	// Draw animated meshes
	animatedShader->Bind();

	animatedShader->SetMat4("uMatProjection", projection);
	animatedShader->SetMat4("uMatView", view);
	animatedShader->SetFloat3("uCameraPosition", cameraPosition);

	for (RenderSubmission& submission : animatedSubmissions)
	{
		RenderComponent* renderComponent = submission.renderComponent;

		if (renderComponent->faceCullType == FaceCullType::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(renderComponent->faceCullType == FaceCullType::Front ? GL_FRONT : GL_BACK);
		}

		PassSharedData(animatedShader, submission, projection, view);

		for (unsigned int i = 0; i < submission.boneMatricesLength; i++) // Pass bone matrices to shader
		{
			glm::mat4& matrix = submission.boneMatrices[i];
			animatedShader->SetMat4("uBoneMatrices[" + std::to_string(i) + "]", matrix);
		}

		renderComponent->Draw(animatedShader, submission.transform);
	}

	geometryBuffer->Unbind();

	glDisable(GL_MULTISAMPLE);
}

void GeometryPass::PassSharedData(Shader* shader, RenderSubmission& submission, const glm::mat4& projection, const glm::mat4& view)
{
	RenderComponent* renderComponent = submission.renderComponent;

	glm::mat4 projViewModel = projection * view * submission.transform;
	glm::mat4& prevProjViewModel = renderComponent->hasPrevProjViewModel ? renderComponent->projViewModel : projViewModel;
	renderComponent->projViewModel = projViewModel;

	//shader->SetMat4("uMatModel", submission->transform);
	//shader->SetMat4("uMatModelInverseTranspose", glm::inverse(submission->transform));
	shader->SetMat4("uMatProjViewModel", projViewModel);
	shader->SetMat4("uMatPrevProjViewModel", prevProjViewModel);

	shader->SetFloat2("uUVOffset", renderComponent->uvOffset);

	if (renderComponent->castShadowsOn)
	{
		shader->SetFloat("uShadowSoftness", renderComponent->surfaceShadowSoftness);
	}
	else // No shadows
	{
		shader->SetFloat("uShadowSoftness", 0.0f);
	}

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

		renderComponent->ormTexture->BindToSlot(5);
		shader->SetInt("uORMTexture", 5);
	}
	else // We have no material textures
	{
		shader->SetFloat4("uMaterialOverrides", glm::vec4(renderComponent->roughness, renderComponent->metalness, renderComponent->ao, 1.0f));
	}

	ReflectRefractData& rrData = renderComponent->reflectRefractData;
	float rrType = rrData.type == ReflectRefractType::Reflect ? 1.0f : rrData.type == ReflectRefractType::Refract ? 2.0f : 0.0f;
	shader->SetFloat4("uRRInfo", glm::vec4(rrType, rrData.strength, renderComponent->reflectRefractData.refractRatio, 0.0f));

	if (rrData.type != ReflectRefractType::None)
	{
		if (rrData.mapType == ReflectRefractMapType::Environment)
		{
			Renderer::envMap1->BindToSlot(8);
		}
		else
		{
			rrData.customMap->BindToSlot(8);
		}
	}

	shader->SetInt("uRRMap", 8); // This has to be outside of the if because for some strange reason sampling from a cube map that hasn't been set stops everything from rendering, even if the code isn't run
}