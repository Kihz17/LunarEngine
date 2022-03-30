#include "DayNightCycle.h"
#include "Renderer.h"
#include "TextureManager.h"

#include <glm/gtx/rotate_vector.hpp>

constexpr float rotateSpeed = 0.0001f;
constexpr glm::vec3 rotationAxis = glm::vec3(0.2f, 0.0f, 0.8f);

constexpr float cloudBaseBrightness = 1.5f;
constexpr float minCloudBrightness = 0.8f;

constexpr glm::vec3 dayColor = glm::vec3(1.0f);
constexpr glm::vec3 sunsetColor = glm::vec3(0.82f, 0.49f, 0.26f);

DayNightCycle::DayNightCycle()
{

}

void DayNightCycle::OnAttach()
{
	// Setup constant night light
	LightInfo lightInfo;
	lightInfo.intensity = 0.5f;
	lightInfo.color = glm::vec4(1.0f);
	lightInfo.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	lightInfo.lightType = LightType::Directional;
	nightLight = new Light(lightInfo);


	std::vector<std::string> paths;
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	CubeMap* envMap2 = TextureManager::CreateCubeMap(paths, TextureFilterType::Linear, TextureWrapType::Repeat, true, false);
	Renderer::envMap2 = envMap2;
}

void DayNightCycle::OnUpdate(float deltaTime)
{
	Light* mainLight = Renderer::GetMainLightSource();

	// Rotate sun around
	glm::vec3 lightDir = glm::normalize(mainLight->direction);
	glm::vec3 newDir = glm::rotate(lightDir, rotateSpeed, rotationAxis);
	mainLight->UpdateDirection(newDir);

	// Make light darker as sun goes down
	float intensity = newDir.y > 0.0f ? 0.0f : glm::clamp(20.0f * glm::pow(glm::abs(newDir.y), 1.5f), 0.0f, 20.0f);
	mainLight->UpdateIntenisty(intensity);

	// Make clouds darker as sun goes down
	float cloudDarknessScale = glm::clamp(glm::abs(newDir.y) + 0.5f, 0.0f, 1.0f);
	Renderer::GetCloudPass()->cloudDarkness = newDir.y > 0.0f ? minCloudBrightness : glm::clamp(cloudBaseBrightness * cloudDarknessScale, minCloudBrightness, cloudBaseBrightness);

	// Skybox transition
	Renderer::environmentMixFactors.x = glm::clamp(newDir.y > 0.0f ? 1.0f : 1.0f - glm::abs(newDir.y), 0.0f, 1.0f);

	// Sun color change
	glm::vec3 newColor = glm::mix(dayColor, sunsetColor, 1.0f - glm::abs(newDir.y));
	Renderer::GetMainLightSource()->UpdateColor(newColor);
}