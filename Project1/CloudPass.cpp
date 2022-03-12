#include "CloudPass.h"
#include "ShaderLibrary.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"

const std::string CloudPass::CLOUD_SHADER_KEY = "cloudShader";



CloudPass::CloudPass(const WindowSpecs* windowSpecs)
	: frameBuffer(new FrameBuffer()),
	shader(ShaderLibrary::Load(CLOUD_SHADER_KEY, "assets/shaders/clouds.glsl")),
	cloudAttachment(TextureManager::CreateTexture2D(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowSpecs->width, windowSpecs->height, TextureFilterType::Nearest, TextureWrapType::None)),
	shapeNoise(nullptr),
	detailNoise(nullptr),
	offsetTexture(nullptr)
{
	frameBuffer->Bind();
	frameBuffer->AddColorAttachment2D("cloudBuffer", cloudAttachment, 0);
	frameBuffer->Unbind();

	shader->Bind();
	shader->InitializeUniform("uMatView");
	shader->InitializeUniform("uMatProjection");
	shader->InitializeUniform("uBoxCenter");
	shader->InitializeUniform("uBoxExtents");
	shader->InitializeUniform("uPositionBuffer");
	shader->InitializeUniform("uLightDirection");
	shader->InitializeUniform("uLightColor");
	shader->InitializeUniform("uShapeNoise");
	shader->InitializeUniform("uDetailNoise");
	shader->InitializeUniform("uOffsetTexture");
	shader->InitializeUniform("uPositionBuffer");
	shader->InitializeUniform("uScreenDimesnions");
	shader->InitializeUniform("uLightStepCount");
	shader->InitializeUniform("uRayOffsetStrength");
	shader->InitializeUniform("uRayOffsetStrength");
	shader->InitializeUniform("uCloudScale");
	shader->InitializeUniform("uDensityMultiplier");
	shader->InitializeUniform("uDensityOffset");
	shader->InitializeUniform("uShapeOffset");
	shader->InitializeUniform("uShapeNoiseWeights");
	shader->InitializeUniform("uPhaseParams");
	shader->InitializeUniform("uLightAbsorptionThroughCloud");
	shader->InitializeUniform("uLightAbsorptionTowardsSun");
	shader->InitializeUniform("uDarknessThreshold");
	shader->InitializeUniform("uDetailNoiseScale");
	shader->InitializeUniform("uDetailNoiseWeight");
	shader->InitializeUniform("uDetailNoiseWeights");
	shader->InitializeUniform("uDetailOffset");
	shader->InitializeUniform("uTimeScale");
	shader->InitializeUniform("uTime");
	shader->InitializeUniform("uSpeed");
	shader->InitializeUniform("uDetailSpeed");
	shader->Unbind();
}

CloudPass::~CloudPass()
{
	delete frameBuffer;
}

void CloudPass::DoPass(ITexture* positionBuffer, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& cameraDir, PrimitiveShape* quad, const WindowSpecs* windowSpecs)
{
	glDisable(GL_DEPTH_TEST); // Disable depth buffer so that the quad doesnt get discarded

	//frameBuffer->Bind();

	shader->Bind();

	shader->SetMat4("uMatProjection", projection);
	shader->SetMat4("uMatView", view);

	shader->SetFloat3("uBoxCenter", glm::vec3(0.0f, 40.0f, 0.0f));
	shader->SetFloat3("uBoxExtents", glm::vec3(20.0f, 10.0f, 20.0f));

	shader->SetFloat3("uLightDirection", glm::vec3(0.0f, -0.99f, 0.01f));
	shader->SetFloat3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

	shapeNoise->BindToSlot(0);
	shader->SetInt("uPositionBuffer", 0);
	detailNoise->BindToSlot(1);
	shader->SetInt("uDetailNoise", 1);
	offsetTexture->BindToSlot(2);
	shader->SetInt("uOffsetTexture", 2);
	positionBuffer->BindToSlot(3);
	shader->SetInt("uPositionBuffer", 3);

	shader->SetFloat2("uScreenDimesnions", glm::vec2(windowSpecs->width, windowSpecs->height));

	shader->SetInt("uLightStepCount", 8);
	shader->SetFloat("uRayOffsetStrength", 10.0f);

	shader->SetFloat("uCloudScale", 0.62f);
	shader->SetFloat("uDensityMultiplier", 1.07f);
	shader->SetFloat("uDensityOffset", -3.43f);
	shader->SetFloat3("uShapeOffset", glm::vec3(190.44f, 0.0f, 0.0f));
	shader->SetFloat4("uShapeNoiseWeights", glm::vec4(0.0f, 0.8f, 0.15f, 0.0f));

	float forwardScattering = 0.83f;
	float backScattering = 0.3f;
	float baseBrightness = 0.8f;
	float phaseFactor = 0.15f;
	shader->SetFloat4("uPhaseParams", glm::vec4(forwardScattering, backScattering, baseBrightness, phaseFactor));

	shader->SetFloat("uLightAbsorptionThroughCloud", 0.54f);
	shader->SetFloat("uLightAbsorptionTowardsSun", 2.05f);
	shader->SetFloat("uDarknessThreshold", 0.3f);

	shader->SetFloat("uDetailNoiseScale", 3.0f);
	shader->SetFloat("uDetailNoiseWeight", 3.42f);
	shader->SetFloat3("uDetailNoiseWeights", glm::vec3(1.0f, 0.5f, 0.5f));
	shader->SetFloat3("uDetailOffset", glm::vec3(51.25f, 0.0f, 0.0f));

	shader->SetFloat("uTimeScale", 1.0f);
	shader->SetFloat("uTime", glfwGetTime());
	shader->SetFloat("uSpeed", 0.5f);
	shader->SetFloat("uDetailSpeed", 1.0f);

	quad->Draw();

	//frameBuffer->Unbind();
}