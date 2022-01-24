#include "Light.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

#include <iostream>
#include <sstream>

int Light::currentLightIndex = 0;
glm::vec3 Light::orientation = glm::vec3(0.0f);
glm::vec3 Light::scale = glm::vec3(1.0f, 1.0f, 1.0f);

std::vector<int> Light::removedLights;

Light::Light(const LightInfo& lightInfo)
	: position(lightInfo.postion),
	direction(lightInfo.direction),
	color(lightInfo.color),
	lightType(lightInfo.lightType),
	radius(lightInfo.radius),
	attenuationMode(lightInfo.attenMode),
	on(lightInfo.on),
	intensity(lightInfo.intensity)
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

	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	positionLoc = std::string(lightHandle + "position");
	directionLoc = std::string(lightHandle + "direction");
	colorLoc = std::string(lightHandle + "color");
	param1Loc = std::string(lightHandle + "param1");

	SendToShader();

	brdfShader->Bind();
	brdfShader->SetInt("uLightAmount", Light::currentLightIndex + 1);

	forwardShader->Bind();
	forwardShader->SetInt("uLightAmount", Light::currentLightIndex + 1);
}

Light::~Light()
{
	UpdateOn(false); // Turn the light off in the shader
	removedLights.push_back(this->lightIndex); // Save the index so it can be used again later
}

void Light::ImGuiUpdate()
{
	if (ImGui::TreeNode("Light"))
	{
		ImGui::DragFloat3("Position", (float*)&position, 0.01f);
		if (lightType == LightType::Directional)
		{
			ImGui::DragFloat3("Direction", (float*)&direction, 0.01f);
		}
		ImGui::ColorEdit3("Diffuse", (float*)&color);
		ImGui::DragFloat("Radius", &radius, 0.01f);
		if (lightType == LightType::Point)
		{
			ImGui::DragFloat("Intensity", &intensity, 0.01f);
		}

		ImGui::NewLine();

		// Atten selection
		if (lightType != LightType::Directional)
		{
			ImGui::Text("Attenuation Mode");
			if (ImGui::RadioButton("Linear", (int*)&attenuationMode, 0))
			{
				attenuationMode = AttenuationMode::Linear;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Quadratic", (int*)&attenuationMode, 1))
			{
				attenuationMode = AttenuationMode::Quadratic;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("UE4", (int*)&attenuationMode, 2))
			{
				attenuationMode = AttenuationMode::UE4;
			}
		}

		ImGui::NewLine();

		// Light Type selection
		ImGui::Text("Light Type");
		if (ImGui::RadioButton("Directional", (int*)&lightType, 0))
		{
			lightType = LightType::Directional;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Point", (int*)&lightType, 1))
		{
			lightType = LightType::Point;
		}

		ImGui::NewLine();
		ImGui::Checkbox("On", &on);

		SendToShader();

		ImGui::TreePop();
	}
}

void Light::UpdatePosition(const glm::vec3& position)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->position = position;

	brdfShader->Bind();
	brdfShader->SetFloat3(positionLoc, position);

	forwardShader->Bind();
	forwardShader->SetFloat3(positionLoc, position);
}

void Light::UpdateDirection(const glm::vec3& direction)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->direction = direction;

	brdfShader->Bind();
	brdfShader->SetFloat3(directionLoc, direction);

	forwardShader->Bind();
	forwardShader->SetFloat3(directionLoc, direction);
}

void Light::UpdateColor(const glm::vec3& color)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->color = color;

	brdfShader->Bind();
	brdfShader->SetFloat4(colorLoc, glm::vec4(color, intensity));

	forwardShader->Bind();
	forwardShader->SetFloat4(colorLoc, glm::vec4(color, intensity));
}

void Light::UpdateIntenisty(float intensity)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->intensity = intensity;

	brdfShader->Bind();
	brdfShader->SetFloat4(colorLoc, glm::vec4(color, intensity));

	forwardShader->Bind();
	forwardShader->SetFloat4(colorLoc, glm::vec4(color, intensity));
}

void Light::UpdateRadius(float radius)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->radius = radius;

	brdfShader->Bind();
	brdfShader->SetFloat4(param1Loc, glm::vec4((GLfloat) lightType, radius, (GLfloat) on, (GLfloat) attenuationMode));

	forwardShader->Bind();
	forwardShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
}

void Light::UpdateOn(bool on)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->on = on;

	brdfShader->Bind();
	brdfShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));

	forwardShader->Bind();
	forwardShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
}

void Light::UpdateAttenuationMode(AttenuationMode attenMode)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->attenuationMode = attenMode;

	brdfShader->Bind();
	brdfShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat) attenuationMode));

	forwardShader->Bind();
	forwardShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
}

void Light::UpdateLightType(LightType lightType)
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	this->lightType = lightType;

	brdfShader->Bind();
	brdfShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));

	forwardShader->Bind();
	forwardShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
}

void Light::SendToShader() const
{
	const Shader* brdfShader = ShaderLibrary::Get(Renderer::LIGHTING_SHADER_KEY);
	const Shader* forwardShader = ShaderLibrary::Get(Renderer::FORWARD_SHADER_KEY);

	brdfShader->Bind();
	brdfShader->SetFloat3(positionLoc, position);
	brdfShader->SetFloat3(directionLoc, direction);
	brdfShader->SetFloat4(colorLoc, glm::vec4(color, intensity));
	brdfShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));

	forwardShader->Bind();
	forwardShader->SetFloat3(positionLoc, position);
	forwardShader->SetFloat3(directionLoc, direction);
	forwardShader->SetFloat4(colorLoc, glm::vec4(color, intensity));
	forwardShader->SetFloat4(param1Loc, glm::vec4((GLfloat)lightType, radius, (GLfloat)on, (GLfloat)attenuationMode));
}