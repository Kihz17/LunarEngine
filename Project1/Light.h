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

class Light
{
public:
	Light(const glm::vec3& postion = glm::vec3(0.0f), const glm::vec3& direction = glm::vec3(0.01f, -0.99f, 0.0f), 
		const glm::vec4& color = glm::vec4(1.0f), LightType lightType = LightType::Point, float radius = 100.0f, AttenuationMode attenMode = AttenuationMode::Linear, bool on = true);
	virtual ~Light();

	void UpdatePosition(const glm::vec3& position);
	void UpdateDirection(const glm::vec3& direction);
	void UpdateColor(const glm::vec4& color);
	void UpdateRadius(float radius);
	void UpdateOn(bool on);
	void UpdateAttenuationMode(AttenuationMode attenMode);
	void UpdateLightType(LightType lightType);
	void SendToShader() const;

	const glm::vec3& GetPosition() const { return position; }
	const glm::vec3& GetDirection() const { return direction; }
	const glm::vec4& GetColor() const { return color; }
	const float& GetRadius() const { return radius; }
	bool IsOn() const { return on; }
	const glm::vec3& GetAttenutationMode() const { return position; }
	LightType GetLightType() const { return lightType; }

	static const int MAX_LIGHTS = 100;

private:
	int lightIndex;
	std::string positionLoc;
	std::string directionLoc;
	std::string colorLoc;
	std::string param1Loc;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec4 color;
	float radius;
	bool on;
	AttenuationMode attenuationMode;
	LightType lightType;

	static int currentLightIndex;
	static std::vector<int> removedLights;
};