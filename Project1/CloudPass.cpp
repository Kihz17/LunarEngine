#include "CloudPass.h"
#include "ShaderLibrary.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "Utils.h"

#include "vendor/imgui/imgui.h"

const std::string CloudPass::CLOUD_SHADER_KEY = "cloudShader";
const std::string CloudPass::WEATHER_SHADER_KEY = "weatherShader";
const std::string CloudPass::PERLIN_WORLEY_SHADER_KEY = "perlinWorleyShader";
const std::string CloudPass::WORLEY_SHADER_KEY = "worleyShader";

CloudPass::CloudPass(const WindowSpecs* windowSpecs)
	: cloudShader(ShaderLibrary::LoadCompute(CLOUD_SHADER_KEY, "assets//volumetricClouds.glsl")),
	weatherShader(ShaderLibrary::LoadCompute(WEATHER_SHADER_KEY, "assets/shaders/weatherGenerator.glsl")),
	perlinWorleyShader(ShaderLibrary::LoadCompute(PERLIN_WORLEY_SHADER_KEY, "assets/shaders/perlinWorleyGenerator.glsl")),
	worleyShader(ShaderLibrary::LoadCompute(WORLEY_SHADER_KEY, "assets/shaders/worleyGenerator.glsl")),
	cloudTexture(nullptr),
	worleyTexture(nullptr),
	weatherTexture(nullptr),
	coverage(0.45f),
	cloudSpeed(450.0f),
	crispiness(40.0f),
	curliness(1.0f),
	density(0.02f),
	absorptionToLight(0.35f),
	earthRadius(600000.0f),
	sphereInnerRadius(5000.0f),
	sphereOuterRadius(17000.0f),
	perlinFreq(0.8f),
	enableGodRays(false),
	cloudColorTop(glm::vec3(169.0f, 149.0f, 149.0f) * (1.5f / 255.0f)),
	cloudColorBottom(glm::vec3(65.0f, 70.0f, 80.0f) * (1.5f / 255.0f)),
	weatherSeed(Utils::RandFloat(0.0f, 100.0f), Utils::RandFloat(0.0f, 100.0f), Utils::RandFloat(0.0f, 100.0f)),
	weatherPerlinFreq(0.8f)
{
	cloudShader->Bind();
	cloudShader->InitializeUniform("uCloudTexture");
	cloudShader->InitializeUniform("uWorleyTexture");
	cloudShader->InitializeUniform("uWeatherTexture");
	cloudShader->InitializeUniform("uResolution");
	cloudShader->InitializeUniform("uInvProj");
	cloudShader->InitializeUniform("uInvView");
	cloudShader->InitializeUniform("uCameraPosition");
	cloudShader->InitializeUniform("uLightDirection");
	cloudShader->InitializeUniform("uLightColor");
	cloudShader->InitializeUniform("uCrispiness");
	cloudShader->InitializeUniform("uDensityFactor");
	cloudShader->InitializeUniform("uCloudCoverageMult");
	cloudShader->InitializeUniform("uCurliness");
	cloudShader->InitializeUniform("uAbsorptionToLight");
	cloudShader->InitializeUniform("uCloudColorTop");
	cloudShader->InitializeUniform("uCloudColorBottom");
	cloudShader->InitializeUniform("uTime");
	cloudShader->InitializeUniform("uCloudSpeed");
	cloudShader->InitializeUniform("uWindDirection");
	cloudShader->InitializeUniform("uNoiseKernel");
	cloudShader->InitializeUniform("uBayerFilter");
	cloudShader->InitializeUniform("uEarthRadius");
	cloudShader->InitializeUniform("uSphereInnerRadius");
	cloudShader->InitializeUniform("uSphereOuterRadius");
	cloudShader->Unbind();

	weatherShader->Bind();
	weatherShader->InitializeUniform("uResolution");
	weatherShader->InitializeUniform("uSeed");
	weatherShader->InitializeUniform("uPerlinScale");
	weatherShader->InitializeUniform("uPerlinFrequency");
	weatherShader->InitializeUniform("uPerlinAmplitude");
	weatherShader->Unbind();
}

CloudPass::~CloudPass()
{
	
}

void CloudPass::DoPass(ITexture* skyTexture, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& lightPos, const WindowSpecs* windowSpecs)
{
	ImGui::Begin("Clouds");
	if (ImGui::TreeNode("Cloud Stuff"))
	{
		//ImGui::DragFloat3("BoxPos", (float*)&boxPos, 0.01f);
		//ImGui::DragFloat3("BoxExtents", (float*)&boxExtents, 0.01f);
		//ImGui::DragFloat3("lightDir", (float*)&lightDir, 0.01f);
		//ImGui::DragFloat3("lightColor", (float*)&lightColor, 0.01f);
		//ImGui::DragInt("Step Count", &stepCount);
		//ImGui::DragFloat("Offset Strength", &offsetStrength, 0.01f);
		//ImGui::DragFloat("Cloud Scale", &cloudScale, 0.01f);
		//ImGui::DragFloat("Density Mult", &densityMult, 0.01f);
		//ImGui::DragFloat("Density Offset", &densityOffset, 0.01f);
		//ImGui::DragFloat4("shape Weights", (float*) & shapeNoiseWeights, 0.01f);
		//ImGui::DragFloat4("phase params", (float*) & phaseParams, 0.01f);
		//ImGui::DragFloat("light ab cloud", &lightAbThroughCloud, 0.01f);
		//ImGui::DragFloat("light ab sun", &lightAbToSun, 0.01f);
		//ImGui::DragFloat("darkness thresh", &darknessThreshold, 0.01f);
		//ImGui::DragFloat("detail scale", &detailScale, 0.01f);
		//ImGui::DragFloat("detail weight", &detailWeight, 0.01f);
		//ImGui::DragFloat3("detail weights", (float*) &detailWeights, 0.01f);
		//ImGui::DragFloat3("detail offset", (float*) & detailOffset, 0.01f);
		//ImGui::DragFloat("speed", &speed, 0.01f);
		//ImGui::DragFloat("DSpeed", &detailSpeed, 0.01f);
	}
	ImGui::End();

	cloudShader->Bind();

	cloudShader->SetFloat2("uResolution", glm::vec2(windowSpecs->width, windowSpecs->height));
	cloudShader->SetFloat("uTime", glfwGetTime());
	cloudShader->SetMat4("uInvProj", glm::inverse(projection));
	cloudShader->SetMat4("uInvView", glm::inverse(view));
	cloudShader->SetFloat3("uCameraPosition", cameraPos);
	cloudShader->SetFloat3("uLightDirection", glm::normalize(lightPos - cameraPos));
	cloudShader->SetFloat3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

	cloudShader->SetFloat("uCloudCoverageMult", coverage);
	cloudShader->SetFloat("uCloudSpeed", cloudSpeed);
	cloudShader->SetFloat("uCrispiness", crispiness);
	cloudShader->SetFloat("uCurliness", curliness);
	cloudShader->SetFloat("uAbsorptionToLight", absorptionToLight);
	cloudShader->SetFloat("uDensityFactor", density);

	cloudShader->SetFloat("uEarthRadius", earthRadius);
	cloudShader->SetFloat("uSphereInnerRadius", sphereInnerRadius);
	cloudShader->SetFloat("uSphereOuterRadius", sphereOuterRadius);

	cloudShader->SetFloat3("uCloudColorTop", cloudColorTop);
	cloudShader->SetFloat3("uCloudColorBottom", cloudColorBottom);

	cloudTexture->BindToSlot(0);
	cloudShader->SetInt("uCloudTexture", 0);

	worleyTexture->BindToSlot(1);
	cloudShader->SetInt("uWorleyTexture", 1);

	weatherTexture->BindToSlot(2);
	cloudShader->SetInt("uWeatherTexture", 2);

	skyTexture->BindToSlot(3);
	cloudShader->SetInt("uSkyTexture", 3);

	glDispatchCompute((int)ceil((float)windowSpecs->width / 16), (int)ceil((float)windowSpecs->height / 16), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CloudPass::GenerateTextures(const glm::ivec3& perlinWorleyDimensions, const glm::ivec3& worleyDimensions, const glm::ivec2& weatherDimensions)
{
	// Generate cloud texture (represents the general shape of the clouds)
	{
		cloudTexture = TextureManager::CreateTexture3D(GL_RGBA8, GL_RGBA, GL_FLOAT, perlinWorleyDimensions.x, perlinWorleyDimensions.y, perlinWorleyDimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		perlinWorleyShader->Bind();
		perlinWorleyShader->SetFloat3();
		perlinWorleyShader->SetInt();
		cloudTexture->BindToSlot(0);
		glBindImageTexture(0, cloudTexture->GetID(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8); // Assign layout = 0 to this texture in compute shader
		glDispatchCompute((int)ceil((float)perlinWorleyDimensions.x / 4), (int)ceil((float)perlinWorleyDimensions.y / 4), (int)ceil((float)perlinWorleyDimensions.z / 4));
		glGenerateMipmap(GL_TEXTURE_3D);
	}

	// Generate normal worley texture
	{
		worleyTexture = TextureManager::CreateTexture3D(GL_RGBA8, GL_RGBA, GL_FLOAT, worleyDimensions.x, worleyDimensions.y, worleyDimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		worleyShader->Bind();
		worleyTexture->BindToSlot(0);
		glBindImageTexture(0, worleyTexture->GetID(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		glDispatchCompute((int)ceil((float)worleyDimensions.x / 4), (int)ceil((float)worleyDimensions.y / 4), (int)ceil((float)worleyDimensions.z / 4));
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	
	// Generate weather noise
	{
		weatherTexture = TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, weatherDimensions.x, weatherDimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		weatherShader->Bind();
		weatherShader->SetFloat2("uResolution", glm::vec2(weatherDimensions));
		weatherShader->SetFloat3("uSeed", weatherSeed);
		weatherShader->SetFloat("uPerlinFrequency", weatherPerlinFreq);
		glDispatchCompute((int)ceil((float)weatherDimensions.x / 8), (int)ceil((float)weatherDimensions.y / 8), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}