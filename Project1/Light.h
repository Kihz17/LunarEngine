#pragma once

#include "Shader.h"
#include "IRenderable.h"

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
		intensity(10.0f)
	{}

	glm::vec3 postion;
	glm::vec3 direction;
	glm::vec4 color;
	float intensity;
	LightType lightType;
	float radius;
	AttenuationMode attenMode;
	bool on;
};

class LightRadius : public IRenderable
{
public:
	LightRadius(glm::vec3* position, float radius) 
		: radius(radius),
		position(position),
		scale(glm::vec3(radius, radius, radius)) {}
	virtual ~LightRadius() = default;

	void UpdateRadius(float radius)
	{
		this->radius = radius;
		this->scale = glm::vec3(radius, radius, radius);
	}

	// Inherited from IRenderable
	virtual glm::vec3* GetRenderPosition() override { return position; }
	virtual glm::vec3* GetRenderScale() override { return &scale; }
	virtual glm::vec3* GetRenderOrientation() override { return &orientation; }

	glm::vec3* position;
	float radius;
	glm::vec3 scale;

	static glm::vec3 orientation;
};

class Light : public IRenderable
{
public:
	virtual ~Light();

	void UpdatePosition(const glm::vec3& position);
	void UpdateDirection(const glm::vec3& direction);
	void UpdateColor(const glm::vec3& color);
	void UpdateIntenisty(float intensity);
	void UpdateRadius(float radius);
	void UpdateOn(bool on);
	void UpdateAttenuationMode(AttenuationMode attenMode);
	void UpdateLightType(LightType lightType);
	void SendToShader() const;

	const glm::vec3& GetPosition() const { return position; }
	const glm::vec3& GetDirection() const { return direction; }
	const glm::vec3& GetColor() const { return color; }

	LightRadius& GetRadiusObject() { return radius; }
	const float& GetRadius() const { return radius.radius; }

	bool IsOn() const { return on; }
	AttenuationMode GetAttenutationMode() const { return attenuationMode; }
	LightType GetLightType() const { return lightType; }
	int GetIndex() const { return lightIndex; }
	float GetIntensity() const { return intensity; }

	// Inherited from IRenderable
	virtual glm::vec3* GetRenderPosition() override { return &position; }
	virtual glm::vec3* GetRenderScale() override { return &Light::scale; }
	virtual glm::vec3* GetRenderOrientation() override { return &Light::orientation; }

	static const int MAX_LIGHTS = 100;
	static glm::vec3 orientation;
	static glm::vec3 scale;

private:
	friend class Renderer; // The renderer should be the only object allowed to create lights because we need to track them
	friend class ScenePanel; // Needs direct access to member variables 

	Light(glm::vec3 postion, const glm::vec3& direction, const glm::vec3& color, LightType lightType, float radius, AttenuationMode attenMode, bool on, float intensity);

	int lightIndex;
	std::string positionLoc;
	std::string directionLoc;
	std::string colorLoc;
	std::string param1Loc;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 color;
	LightRadius radius;
	float intensity;
	bool on;
	AttenuationMode attenuationMode;
	LightType lightType;

	static int currentLightIndex;
	static std::vector<int> removedLights;
};