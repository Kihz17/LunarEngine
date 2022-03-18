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
const std::string CloudPass::CLOUD_POST_SHADER_KEY = "cloudPost";

constexpr unsigned int perlinWorleyGenWorkerSize = 4;
constexpr unsigned int worleyGenWorkerSize = 4;
constexpr unsigned int weatherGenWorkerSize = 8;

CloudPass::CloudPass(const WindowSpecs* windowSpecs)
	: cloudShader(ShaderLibrary::LoadCompute(CLOUD_SHADER_KEY, "assets/shaders/volumetricClouds.glsl")),
	weatherShader(ShaderLibrary::LoadCompute(WEATHER_SHADER_KEY, "assets/shaders/weatherGenerator.glsl")),
	perlinWorleyShader(ShaderLibrary::LoadCompute(PERLIN_WORLEY_SHADER_KEY, "assets/shaders/perlinWorleyGenerator.glsl")),
	worleyShader(ShaderLibrary::LoadCompute(WORLEY_SHADER_KEY, "assets/shaders/worleyGenerator.glsl")),
	cloudTexture(nullptr),
	worleyTexture(nullptr),
	weatherTexture(nullptr),
	colorTexture(TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::Repeat)),
	bloomTexture(TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::Repeat)),
	alphanessTexture(TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::Repeat)),
	cloudDistanceTexture(TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::Repeat)),
	postShader(ShaderLibrary::Load(CLOUD_POST_SHADER_KEY, "assets/shaders/cloudPost.glsl")),
	postFramebuffer(new FrameBuffer()),
	postDepthAttachment(TextureManager::CreateTexture2D(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::None, false)),
	postColorAttachment(TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::None)),
	coverage(0.45f),
	cloudSpeed(450.0f),
	crispiness(40.0f),
	curliness(0.1f),
	density(0.02f),
	absorptionToLight(0.35f),
	cloudDarkness(1.0f),
	earthRadius(600000.0f),
	sphereInnerRadius(5000.0f),
	sphereOuterRadius(17000.0f),
	cloudCloseThreshold(earthRadius / 40.0f),
	cloudMediumThreshold(earthRadius / 15.0f),
	perlinFreq(0.8f),
	godRayDecay(0.98f),
	godRayDensity(0.9f),
	godRayWeight(0.07f),
	godRayExposure(0.45f),
	enableGodRays(true),
	cloudColorTop(glm::vec3(169.0f, 149.0f, 149.0f) * (1.5f / 255.0f)),
	cloudColorBottom(glm::vec3(65.0f, 70.0f, 80.0f) * (1.5f / 255.0f)),
	weatherSeed(Utils::RandFloat(0.0f, 1000.0f), Utils::RandFloat(0.0f, 1000.0f), Utils::RandFloat(0.0f, 1000.0f)),
	weatherPerlinFreq(0.8f)
{
	cloudShader->InitializeUniform("uCloudTexture");
	cloudShader->InitializeUniform("uWorleyTexture");
	cloudShader->InitializeUniform("uWeatherTexture");
	cloudShader->InitializeUniform("uSkyTexture");
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
	cloudShader->InitializeUniform("uCloudDarknessMult");

	weatherShader->InitializeUniform("uResolution");
	weatherShader->InitializeUniform("uSeed");
	weatherShader->InitializeUniform("uPerlinScale");
	weatherShader->InitializeUniform("uPerlinFrequency");
	weatherShader->InitializeUniform("uPerlinAmplitude");

	postShader->InitializeUniform("uCloudsTexture");
	postShader->InitializeUniform("uEmissionTexture");
	postShader->InitializeUniform("uEnableGodRays");
	postShader->InitializeUniform("uLightPosition");
	postShader->InitializeUniform("uLightDotCameraDir");
	postShader->InitializeUniform("uResolution");
	postShader->InitializeUniform("uCloudResolution");
	postShader->InitializeUniform("uRadialBlurParams");

	// Setup post processing fbo
	postFramebuffer->Bind();
	postFramebuffer->AddColorAttachment2D("color", postColorAttachment, 0);
	postFramebuffer->SetDepthAttachment(postDepthAttachment);
	postFramebuffer->Unbind();

	GenerateTextures(glm::ivec3(128, 128, 128), glm::ivec3(32, 32, 32), glm::ivec2(1024, 1024));
}

CloudPass::~CloudPass()
{
	delete postFramebuffer;
}

void CloudPass::DoPass(ITexture* skyTexture, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& cameraDir,
	const WindowSpecs* windowSpecs, PrimitiveShape* quad)
{
	ImGui::Begin("Clouds");
	if (ImGui::TreeNode("Cloud Stuff"))
	{
		ImGui::DragFloat("Coverage", &coverage, 0.01f);
		ImGui::DragFloat("Cloud Speed", &cloudSpeed, 0.01f);
		ImGui::DragFloat("Crispiness", &crispiness, 0.01f);
		ImGui::DragFloat("Curliness", &curliness, 0.01f);
		ImGui::DragFloat("Density", &density, 0.00001f, 0.0f, 1.0f, "%.4f");
		ImGui::DragFloat("Absorp", &absorptionToLight, 0.01f);
		ImGui::DragFloat("Darkness", &cloudDarkness, 0.001f);
		ImGui::DragFloat("Earth R", &earthRadius, 0.01f);
		ImGui::DragFloat("Inner R", &sphereInnerRadius, 0.01f);
		ImGui::DragFloat("Outer R", &sphereOuterRadius, 0.01f);
		ImGui::DragFloat("Perlin Freq", &perlinFreq, 0.01f);
		ImGui::DragFloat("Cloud Close", &cloudCloseThreshold, 1.0f);
		ImGui::DragFloat("Cloud Med", &cloudMediumThreshold, 1.0f);
		ImGui::Checkbox("Enable Godrays", &enableGodRays);
		ImGui::DragFloat("God Ray Decay", &godRayDecay, 0.01f);
		ImGui::DragFloat("God Ray Density", &godRayDensity, 0.01f);
		ImGui::DragFloat("God Ray Weight", &godRayWeight, 0.01f);
		ImGui::DragFloat("God Ray Exposure", &godRayExposure, 0.01f);
		ImGui::TreePop();
	}
	ImGui::End();

	cloudShader->Bind();

	// Bind textures for writing so the compute shader can write to them
	glBindImageTexture(0, colorTexture->GetID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, bloomTexture->GetID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, alphanessTexture->GetID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(3, cloudDistanceTexture->GetID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glm::vec3 lightPos = -lightDir * earthRadius;
	glm::vec3 cameraToLightDir = glm::normalize(lightPos - cameraPos);
	glm::vec2 windowResolution(windowSpecs->width, windowSpecs->height);

	cloudShader->SetFloat2("uResolution", windowResolution);
	cloudShader->SetFloat("uTime", glfwGetTime());
	cloudShader->SetMat4("uInvProj", glm::inverse(projection));
	cloudShader->SetMat4("uInvView", glm::inverse(view));
	cloudShader->SetFloat3("uCameraPosition", cameraPos);
	cloudShader->SetFloat3("uLightDirection", cameraToLightDir);
	cloudShader->SetFloat3("uLightColor", lightColor);

	cloudShader->SetFloat("uCloudCoverageMult", coverage);
	cloudShader->SetFloat("uCloudSpeed", cloudSpeed);
	cloudShader->SetFloat("uCrispiness", crispiness);
	cloudShader->SetFloat("uCurliness", curliness);
	cloudShader->SetFloat("uAbsorptionToLight", absorptionToLight * 0.01f);
	cloudShader->SetFloat("uDensityFactor", density);
	cloudShader->SetFloat("uCloudDarknessMult", cloudDarkness);

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
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Ensures that anything after this line will get the updated data from the compute shader

	// Cloud post processing (helps make clouds less pixelated by using gaussian blur and godrays)
	glDisable(GL_DEPTH_TEST);

	postFramebuffer->Bind();
	postShader->Bind();

	colorTexture->BindToSlot(0);
	postShader->SetInt("uCloudsTexture", 0);

	bloomTexture->BindToSlot(1);
	postShader->SetInt("uEmissionTexture", 1);

	postShader->SetInt("uEnableGodRays", (GLboolean) enableGodRays);
	
	// Convert light position to screen space
	glm::mat4 lightMat = glm::translate(glm::mat4(1.0f), lightPos);
	glm::vec4 lightScreenPos = (projection * view) * lightMat * glm::vec4(0.0f, 60.0f, 0.0f, 1.0f);
	lightScreenPos /= lightScreenPos.w;
	lightScreenPos = lightScreenPos * 0.5f + 0.5f;
	postShader->SetFloat3("uLightPosition", glm::vec3(lightScreenPos));

	float lightDotCameraFront = glm::dot(cameraToLightDir, glm::normalize(cameraDir));
	postShader->SetFloat("uLightDotCameraDir", lightDotCameraFront);

	postShader->SetFloat2("uResolution", windowResolution);
	postShader->SetFloat2("uCloudResolution", windowResolution);
	postShader->SetFloat4("uRadialBlurParams", glm::vec4(godRayDecay, godRayDensity, godRayWeight, godRayExposure));

	quad->Draw();

	postFramebuffer->Unbind();
}

void CloudPass::GenerateTextures(const glm::ivec3& perlinWorleyDimensions, const glm::ivec3& worleyDimensions, const glm::ivec2& weatherDimensions)
{
	// Generate cloud texture (represents the general shape of the clouds)
	{
		cloudTexture = TextureManager::CreateTexture3D(GL_RGBA8, GL_RGBA, GL_FLOAT, perlinWorleyDimensions.x, perlinWorleyDimensions.y, perlinWorleyDimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		perlinWorleyShader->Bind();
		cloudTexture->BindToSlot(0);
		glBindImageTexture(0, cloudTexture->GetID(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8); // Assign layout = 0 to this texture in compute shader
		glDispatchCompute((int)ceil((float)perlinWorleyDimensions.x / perlinWorleyGenWorkerSize), 
			(int)ceil((float)perlinWorleyDimensions.y / perlinWorleyGenWorkerSize), 
			(int)ceil((float)perlinWorleyDimensions.z / perlinWorleyGenWorkerSize));
		glGenerateMipmap(GL_TEXTURE_3D);
	}

	// Generate normal worley texture
	{
		worleyTexture = TextureManager::CreateTexture3D(GL_RGBA8, GL_RGBA, GL_FLOAT, worleyDimensions.x, worleyDimensions.y, worleyDimensions.z, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		worleyShader->Bind();
		worleyTexture->BindToSlot(0);
		glBindImageTexture(0, worleyTexture->GetID(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		glDispatchCompute((int)ceil((float)worleyDimensions.x / worleyGenWorkerSize), 
			(int)ceil((float)worleyDimensions.y / worleyGenWorkerSize), 
			(int)ceil((float)worleyDimensions.z / worleyGenWorkerSize));
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	
	// Generate weather noise
	{
		weatherTexture = TextureManager::CreateTexture2D(GL_RGBA32F, GL_RGBA, GL_FLOAT, weatherDimensions.x, weatherDimensions.y, TextureFilterType::Linear, TextureWrapType::Repeat, true);
		weatherShader->Bind();
		weatherShader->SetFloat2("uResolution", glm::vec2(weatherDimensions));
		weatherShader->SetFloat3("uSeed", weatherSeed);
		weatherShader->SetFloat("uPerlinFrequency", weatherPerlinFreq);
		glBindImageTexture(0, weatherTexture->GetID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute((int)ceil((float)weatherDimensions.x / weatherGenWorkerSize), (int)ceil((float)weatherDimensions.y / weatherGenWorkerSize), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Ensures that anything after this line will get the update data from the compute shader
	}
}