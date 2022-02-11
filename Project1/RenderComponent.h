#pragma once

#include "Component.h"
#include "VertexArrayObject.h"
#include "ITexture.h"
#include "CubeMap.h"

enum class RRType
{
	None,
	Reflect,
	Refract
};

struct RenderComponent : public Component
{
	struct RenderInfo
	{
		VertexArrayObject* vao = nullptr;
		uint32_t indexCount = 0;

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

		CubeMap* reflectRefractMap = nullptr;
		RRType reflectRefractType = RRType::None;
		float reflectRefractStrength = 0.0f;
		float refractRatio = 0.0f;
	};

	RenderComponent()
		: vao(nullptr),
		indexCount(0),
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
		reflectRefractMap(nullptr),
		reflectRefractType(RRType::None),
		reflectRefractStrength(0.0f),
		refractRatio(0.0f)
	{}

	RenderComponent(const RenderInfo& renderInfo)
		: vao(renderInfo.vao),
		indexCount(renderInfo.indexCount),
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
		reflectRefractMap(renderInfo.reflectRefractMap),
		reflectRefractType(renderInfo.reflectRefractType),
		reflectRefractStrength(renderInfo.reflectRefractStrength),
		refractRatio(renderInfo.refractRatio)
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
			
			ImGui::NewLine();
			ImGui::DragFloat("Reflect/Refract Strength", &reflectRefractStrength, 0.01f, 0.0f, 1.0f);

			ImGui::Text("Reflection/Refraction Type:");
			int renderSelection = (int) reflectRefractType;
			ImGui::RadioButton("None", &renderSelection, 0); ImGui::SameLine();
			ImGui::RadioButton("Reflect", &renderSelection, 1); ImGui::SameLine();
			ImGui::RadioButton("Refract", &renderSelection, 2);

			if (renderSelection != (int)reflectRefractType)
			{
				reflectRefractType = (RRType) renderSelection;
			}

			ImGui::TreePop();
		}

	}

	bool HasMaterialTextures() const
	{
		return roughnessTexture && metalTexture && aoTexture;
	}

	VertexArrayObject* vao;
	uint32_t indexCount;

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

	CubeMap* reflectRefractMap;
	RRType reflectRefractType;
	float reflectRefractStrength;
	float refractRatio;
};