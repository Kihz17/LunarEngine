#pragma once

#include "Mesh.h"
#include "Component.h"
#include "ITexture.h"
#include "ReflectRefract.h"

struct RenderComponent : public Component
{
	struct RenderInfo
	{
		Mesh* mesh = nullptr;

		bool isColorOverride = false;
		glm::vec3 colorOverride = glm::vec3(0.0f);
		std::vector<std::pair<ITexture*, float>> albedoTextures;

		ITexture* normalTexture = nullptr;
		ITexture* roughnessTexture = nullptr;
		ITexture* metalTexture = nullptr;
		ITexture* aoTexture = nullptr;

		float roughness = 0.01f;
		float metalness = 0.02f;
		float ao = 1.0f;

		bool isWireframe = false;
		bool isIgnoreLighting = false;

		float alphaTransparency = 1.0f;

		bool castShadows = true;
		bool castShadowsOn = true;
		float shadowSoftness = 1.0f;

		ReflectRefractType reflectRefractType = ReflectRefractType::None;
		ReflectRefractMapType reflectRefractMapType = ReflectRefractMapType::Environment;
		CubeMap* reflectRefractCustomMap = nullptr;
		float reflectRefractStrength = 0.0f;
		float refractRatio = 0.0f;
		ReflectRefractMapPriorityType reflectRefractMapPriority = ReflectRefractMapPriorityType::High;
	};

	RenderComponent()
		: mesh(nullptr),
		isColorOverride(false),
		colorOverride(glm::vec3(0.0f)),
		normalTexture(nullptr),
		roughnessTexture(nullptr),
		metalTexture(nullptr),
		aoTexture(nullptr),
		roughness(0.01f),
		metalness(0.02f),
		ao(1.0f),
		isWireframe(false),
		isIgnoreLighting(false),
		alphaTransparency(1.0f),
		hasPrevProjViewModel(false),
		projViewModel(glm::mat4(1.0f)),
		castShadows(true),
		castShadowsOn(true),
		shadowSoftness(1.0f),
		reflectRefractMapPriority(ReflectRefractMapPriorityType::High)
	{}

	RenderComponent(const RenderInfo& renderInfo)
		: mesh(renderInfo.mesh),
		isColorOverride(renderInfo.isColorOverride),
		colorOverride(renderInfo.colorOverride),
		albedoTextures(renderInfo.albedoTextures),
		normalTexture(renderInfo.normalTexture),
		roughnessTexture(renderInfo.roughnessTexture),
		metalTexture(renderInfo.metalTexture),
		aoTexture(renderInfo.aoTexture),
		roughness(renderInfo.roughness),
		metalness(renderInfo.metalness),
		ao(renderInfo.ao),
		isWireframe(renderInfo.isWireframe),
		isIgnoreLighting(renderInfo.isIgnoreLighting),
		alphaTransparency(renderInfo.alphaTransparency),
		hasPrevProjViewModel(false),
		projViewModel(glm::mat4(1.0f)),
		castShadows(renderInfo.castShadows),
		castShadowsOn(renderInfo.castShadowsOn),
		shadowSoftness(renderInfo.shadowSoftness),
		reflectRefractData(renderInfo.reflectRefractType, renderInfo.reflectRefractMapType, renderInfo.reflectRefractCustomMap, renderInfo.reflectRefractStrength, renderInfo.refractRatio),
		reflectRefractMapPriority(renderInfo.reflectRefractMapPriority)
	{}

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Render"))
		{
			// Diffuse Color
			if (isColorOverride)
			{
				ImGui::ColorEdit3("Color", (float*)&colorOverride);
			}
			else
			{
				// TODO: Allow changing albedo textures and ratios
			}

			ImGui::NewLine();

			// Materials
			if (!HasMaterialTextures())
			{
				ImGui::DragFloat("Roughness", &roughness, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("Metalness", &metalness, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("Ambient Occlusion", &ao, 0.001f, 0.0f, 1.0f);
			}
			else
			{
				// TODO: Change material textures
			}

			ImGui::NewLine();

			ImGui::DragFloat("Transparency", &alphaTransparency, 0.001f, 0.0f, 1.0f);
			ImGui::Checkbox("Ignore Lighting", &isIgnoreLighting);
			ImGui::Checkbox("Cast Shadows", &castShadows);
			ImGui::Checkbox("Cast Shadows On", &castShadowsOn);
			ImGui::DragFloat("Shadow Softness", &shadowSoftness, 0.001f, 0.0f, 1.0f);
			
			ImGui::NewLine();
			ImGui::DragFloat("Reflect/Refract Strength", &reflectRefractData.strength, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Refract Ratio", &reflectRefractData.refractRatio, 0.001f);

			ImGui::Text("Reflection/Refraction Type:");
			int renderSelection = (int) reflectRefractData.type;
			ImGui::RadioButton("None", &renderSelection, 0); ImGui::SameLine();
			ImGui::RadioButton("Reflect", &renderSelection, 1); ImGui::SameLine();
			ImGui::RadioButton("Refract", &renderSelection, 2);

			if (renderSelection != (int)reflectRefractData.type)
			{
				reflectRefractData.type = (ReflectRefractType) renderSelection;
			}

			ImGui::NewLine();
			ImGui::Text("Reflection/Refraction Map Type:");
			int rrMapTypeSelection = (int) reflectRefractData.mapType;
			ImGui::RadioButton("Environment", &rrMapTypeSelection, 0);
			ImGui::RadioButton("Dynamic Minimal", &rrMapTypeSelection, 1);
			ImGui::RadioButton("Dynamic Medium", &rrMapTypeSelection, 2); 
			ImGui::RadioButton("Dynamic Full", &rrMapTypeSelection, 3);

			if (rrMapTypeSelection != (int)reflectRefractData.mapType)
			{
				reflectRefractData.mapType = (ReflectRefractMapType) rrMapTypeSelection;
			}

			ImGui::TreePop();
		}

	}

	bool HasMaterialTextures() const
	{
		return roughnessTexture && metalTexture && aoTexture;
	}

	Mesh* mesh;

	// Diffuse color
	bool isColorOverride;
	glm::vec3 colorOverride;
	std::vector<std::pair<ITexture*, float>> albedoTextures;

	// Materials Textures
	ITexture* normalTexture;
	ITexture* roughnessTexture;
	ITexture* metalTexture;
	ITexture* aoTexture;

	// Material values (should only be used if material textures are not present)
	float roughness;
	float metalness;
	float ao;

	bool isWireframe;
	bool isIgnoreLighting;

	float alphaTransparency;

	bool hasPrevProjViewModel;
	glm::mat4 projViewModel;

	bool castShadows;
	bool castShadowsOn;
	float shadowSoftness;

	ReflectRefractData reflectRefractData;
	ReflectRefractMapPriorityType reflectRefractMapPriority;
};