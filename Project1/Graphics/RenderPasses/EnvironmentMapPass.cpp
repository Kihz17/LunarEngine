#include "EnvironmentMapPass.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "ShaderLibrary.h"
#include "RenderBuffer.h"
#include "Renderer.h"

const std::string EnvironmentMapPass::CUBE_MAP_DRAW_SHADER_KEY = "drawEnvShader";

EnvironmentMapPass::EnvironmentMapPass(const WindowSpecs* windowSpecs)
	: environmentBuffer(new FrameBuffer()),
	shader(ShaderLibrary::Load(CUBE_MAP_DRAW_SHADER_KEY, "assets/shaders/environmentBuffer.glsl")),
	envMapTexture(TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::None)),
	windowSpecs(windowSpecs)
{
	// Setup color attachements for FBO
	environmentBuffer->Bind();
	environmentBuffer->AddColorAttachment2D("environment", envMapTexture, 0);
	environmentBuffer->Unbind();

	// Setup shader uniforms
	shader->InitializeUniform("uProjection");
	shader->InitializeUniform("uView");
	shader->InitializeUniform("uEnvMap1");
	shader->InitializeUniform("uEnvMap2");
	shader->InitializeUniform("uResolution");
	shader->InitializeUniform("uLightDirection");
	shader->InitializeUniform("uLightColor");
	shader->InitializeUniform("uInvProj");
	shader->InitializeUniform("uInvView");
	shader->InitializeUniform("uMixFactors");

	shader->Bind();
	shader->SetInt("uEnvMap1", 0);
	shader->SetInt("uEnvMap2", 1);
	shader->Unbind();
}

EnvironmentMapPass::~EnvironmentMapPass()
{
	delete environmentBuffer;
}

void EnvironmentMapPass::DoPass(CubeMap* cubeMap1, CubeMap* cubeMap2, const glm::vec4& mixFactors, const glm::mat4& projection, const glm::mat4& view, bool sun, const glm::vec3& cameraPos, const glm::vec3& lightDir, const glm::vec3& lightColor, PrimitiveShape* cube)
{
	glDisable(GL_CULL_FACE); // Make sure none of our cubes faces get culled

	environmentBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->Bind();

	shader->SetMat4("uProjection", projection);
	shader->SetMat4("uView", view);
	shader->SetMat4("uInvProj", glm::inverse(projection));
	shader->SetMat4("uInvView", glm::inverse(view));

	shader->SetFloat2("uResolution", glm::vec2(envMapTexture->GetWidth(), envMapTexture->GetHeight()));

	if (sun)
	{
		glm::vec3 lightPos = -lightDir * 600000.0f;
		shader->SetFloat3("uLightColor", lightColor);
		shader->SetFloat4("uLightDirection", glm::vec4(glm::normalize(lightPos - cameraPos), 1.0f));
	}
	else
	{
		shader->SetFloat4("uLightDirection", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	
	shader->SetFloat4("uMixFactors", mixFactors);

	cubeMap1->BindToSlot(0);
	shader->SetInt("uEnvMap1", 0);

	if (cubeMap2)
	{
		cubeMap2->BindToSlot(1);
	}

	shader->SetInt("uEnvMap2", 1);

	cube->Draw();

	environmentBuffer->Unbind();

	glEnable(GL_CULL_FACE); // Re-enable face culling
}