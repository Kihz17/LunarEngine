#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#include <vector>

enum class LightType
{
	Directional,
	Point,
	IBL
};

enum class AttenuationMode
{
	Linear,
	Quadratic,
	UE4
};

struct LightInfo
{
	LightInfo()
		: postion(glm::vec3(0.0f)),
		direction(glm::vec3(0.01f, -0.99f, 0.0f)),
		color(glm::vec4(1.0f)),
		lightType(LightType::Point),
		radius(100.0f),
		attenMode(AttenuationMode::Linear),
		on(true),
		intensity(20.0f),
		castShadows(false)
	{}

	glm::vec3 postion;
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
	LightType lightType;
	float radius;
	AttenuationMode attenMode;
	bool on;
	bool castShadows;
};

class Light
{
public:
	Light(const LightInfo& lightInfo);
	virtual ~Light();

	void UpdatePosition(const glm::vec3& position);
	void UpdateDirection(const glm::vec3& direction);
	void UpdateColor(const glm::vec3& color);
	void UpdateIntenisty(float intensity);
	void UpdateRadius(float radius);
	void UpdateOn(bool on);
	void UpdateAttenuationMode(AttenuationMode attenMode);
	void UpdateLightType(LightType lightType);
	void UpdateCastShadows(bool castShadows);
	void SendToShader() const;

	const glm::vec3& GetPosition() const { return position; }
	const glm::vec3& GetDirection() const { return direction; }
	const glm::vec3& GetColor() const { return color; }

	const float& GetRadius() const { return radius; }

	bool IsOn() const { return on; }
	AttenuationMode GetAttenutationMode() const { return attenuationMode; }
	LightType GetLightType() const { return lightType; }
	int GetIndex() const { return lightIndex; }
	float GetIntensity() const { return intensity; }

	static const int MAX_LIGHTS = 100;

	int lightIndex;
	std::string positionLoc;
	std::string directionLoc;
	std::string colorLoc;
	std::string param1Loc;
	std::string castShadowsLoc;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 color;
	float radius;
	float intensity;
	bool on;
	AttenuationMode attenuationMode;
	LightType lightType;
	bool castShadows;

	static int currentLightIndex;
	static std::vector<int> removedLights;
};