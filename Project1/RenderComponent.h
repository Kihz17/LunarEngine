#pragma once

#include "Component.h"
#include "VertexArrayObject.h"
#include "ITexture.h"

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
		castShadowsOn(true)
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
		castShadowsOn(renderInfo.castShadowsOn)
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
};