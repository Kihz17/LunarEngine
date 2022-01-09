#include "Light.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

#include <iostream>
#include <sstream>

int Light::currentLightIndex = 0;
std::vector<int> Light::removedLights;

Light::Light(const glm::vec3& postion, const glm::vec3& direction, const glm::vec4& color, LightType lightType, float radius, AttenuationMode attenMode, bool on)
	: position(position),
	direction(direction),
	color(color),
	lightType(lightType),
	radius(radius),
	attenuationMode(attenMode),
	on(on)
{
	if (Light::currentLightIndex >= 1000 && removedLights.empty())
	{
		std::cout << "Cannot create more than 1000 lights!" << std::endl;
		return;
	}

	if (!removedLights.empty())
	{
		this->lightIndex = Light::removedLights[0];
		removedLights.erase(removedLights.begin());
	}
	else
	{
		this->lightIndex = Light::currentLightIndex++;
	}

	// Initialize uniforms
	std::stringstream ss;
	ss << "uLightArray[" << this->lightIndex << "].";
	std::string lightHandle = ss.str();

	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	positionLoc = std::string(lightHandle + "position");
	directionLoc = std::string(lightHandle + "direction");
	colorLoc = std::string(lightHandle + "color");
	param1Loc = std::string(lightHandle + "param1");

	SendToShader();

	shader->Bind();
	shader->SetInt("uLightAmount", Light::currentLightIndex + 1);
}

Light::~Light()
{
	UpdateOn(false); // Turn the light off in the shader
	removedLights.push_back(this->lightIndex); // Save the index so it can be used again later
}

void Light::UpdatePosition(const glm::vec3& position)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->position = position;
	shader->SetFloat3(positionLoc, position);
	shader->Unbind();
}

void Light::UpdateDirection(const glm::vec3& direction)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->direction = direction;
	shader->SetFloat3(directionLoc, direction);
	shader->Unbind();
}

void Light::UpdateColor(const glm::vec4& color)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->color = color;
	shader->SetFloat4(colorLoc, color);
	shader->Unbind();
}

void Light::UpdateRadius(float radius)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->radius = radius;
	shader->SetFloat4(param1Loc, glm::vec4((GLfloat) lightType, radius, (GLfloat) on, (GLfloat) attenuationMode));
	shader->Unbind();
}

void Light::UpdateOn(bool on)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->on = on;
	shader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
	shader->Unbind();
}

void Light::UpdateAttenuationMode(AttenuationMode attenMode)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->attenuationMode = attenMode;
	shader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat) attenuationMode));
	shader->Unbind();
}

void Light::UpdateLightType(LightType lightType)
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	this->lightType = lightType;
	shader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
	shader->Unbind();
}

void Light::SendToShader() const
{
	const Shader* shader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	shader->Bind();
	shader->SetFloat3(positionLoc, position);
	shader->SetFloat3(directionLoc, direction);
	shader->SetFloat4(colorLoc, color);
	shader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
	shader->Unbind();
}