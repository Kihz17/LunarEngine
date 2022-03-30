#include "DayNightCycle.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "TagComponent.h"
#include "LightComponent.h"

#include <glm/gtx/rotate_vector.hpp>

constexpr float rotateSpeed = 0.0001f;
constexpr glm::vec3 rotationAxis = glm::vec3(0.2f, 0.0f, 0.8f);

constexpr float cloudBaseBrightness = 1.5f;
constexpr float minCloudBrightness = 0.8f;

constexpr glm::vec3 dayColor = glm::vec3(1.0f);
constexpr glm::vec3 sunsetColor = glm::vec3(0.82f, 0.49f, 0.26f);

constexpr float baseLanternIntensity = 150.0f;
const std::string lanternTag("laternEntity");

DayNightCycle::DayNightCycle(EntityManager& entityManager)
	: entityManager(entityManager)
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

	// Add "Night time" skybox
	std::vector<std::string> paths;
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	paths.push_back("assets/textures/simpleSkyDark.png");
	CubeMap* envMap2 = TextureManager::CreateCubeMap(paths, TextureFilterType::Linear, TextureWrapType::Repeat, true, false);
	Renderer::envMap2 = envMap2;

	// Find laterns
	for (Entity* e : entityManager.GetEntities())
	{
		TagComponent* tagComp = e->GetComponent<TagComponent>();
		if (!tagComp || !tagComp->HasTag(lanternTag)) continue;

		LightComponent* lightComp = e->GetComponent<LightComponent>();
		if (!lightComp) continue;

		lanterns.push_back(lightComp->ptr);
	}
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

	// Adjust latern brightness depending on time of day and flicker
	constexpr float lanternStart = -0.3f; // Point at which we should begin lantern illuminaiton
	constexpr float lanternEnd = 0.0f; // Point at which lanterns should be fully lit
	constexpr float lanternDivisor = glm::abs(lanternStart - lanternEnd);

	float lanternBrightnessContrib = 0.0f;
	if (newDir.y > lanternEnd) // Sun is low, fully illuminate
	{
		lanternBrightnessContrib = 1.0f;
	}
	else if(newDir.y < lanternStart) // sun is to high, no illumination
	{
		lanternBrightnessContrib = 0.0f;
	}
	else
	{
		lanternBrightnessContrib = 1.0f - (glm::abs(newDir.y) / lanternDivisor);
	}

	//lanternBrightnessContrib *= Utils::RandFloat(0.4f, 1.0f); // Add some varience to the contrib to simulate flickering

	for (Light* light : lanterns)
	{
		light->UpdateIntenisty(baseLanternIntensity * lanternBrightnessContrib);
	}
}