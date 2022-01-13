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
		on(true)
	{}

	glm::vec3 postion;
	glm::vec3 direction;
	glm::vec4 color;
	LightType lightType;
	float radius;
	AttenuationMode attenMode;
	bool on;
};

class Light
{
public:
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
	int GetIndex() const { return lightIndex; }

	static const int MAX_LIGHTS = 100;

private:
	friend class Renderer; // The renderer should be the only object allowed to create lights because we need to track them
	friend class ScenePanel; // Needs direct access to member variables 

	Light(const glm::vec3& postion, const glm::vec3& direction, const glm::vec4& color, LightType lightType, float radius, AttenuationMode attenMode, bool on);

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