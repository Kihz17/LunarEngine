#pragma once

#include "VertexArrayObject.h"
#include "Texture.h"

#include <IRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "vendor/imgui/imgui.h"

#include <string>

struct Component
{
	virtual ~Component() = default;

	virtual void ImGuiUpdate() = 0;

protected:
	Component() = default;
};

struct Position : public Component
{
	Position(glm::vec3 value) : value(value) {}
	Position() : value(glm::vec3(0.0f)) {}

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Position Component"))
		{
			ImGui::DragFloat3("Position", (float*)&value, 0.01f);
			ImGui::TreePop();
		}
	}

	glm::vec3 value;
};

struct Rotation : public Component
{
	Rotation() : value(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {}

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Rotation"))
		{
			ImGui::DragFloat4("Rotation", (float*)&value, 0.01f);
			ImGui::TreePop();
		}
	}

	glm::quat value;
};

struct Scale : public Component
{
	Scale(glm::vec3 value) : value(value) {}
	Scale() : value(glm::vec3(1.0f)) {}
	
	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Scale"))
		{
			ImGui::DragFloat3("Scale", (float*)&value, 0.01f);
			ImGui::TreePop();
		}
	}

	glm::vec3 value;
};

struct Velocity : public Component
{
	Velocity() : value(glm::vec3(0.0f)) {}

	virtual void ImGuiUpdate() override
	{
	
	}

	glm::vec3 value;
};

struct RigidBody: public Component
{
	virtual void ImGuiUpdate() override
	{

	}

	Physics::IRigidBody* ptr;
};

struct Render : public Component
{
	struct RenderInfo
	{
		VertexArrayObject* vao = nullptr;
		uint32_t indexCount = 0;

		bool isColorOverride = false;
		glm::vec3 colorOverride = glm::vec3(0.0f);
		std::vector<std::pair<Texture*, float>> albedoTextures;

		Texture* normalTexture = nullptr;
		Texture* roughnessTexture = nullptr;
		Texture* metalTexture = nullptr;
		Texture* aoTexture = nullptr;

		float roughness = 0.01f;
		float metalness = 0.02f;
		float ao = 1.0f;

		bool isWireframe = false;
		bool isIgnoreLighting = false;

		float alphaTransparency = 1.0f;
	};

	Render()
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
		projViewModel(glm::mat4(1.0f))
	{}

	Render (const RenderInfo& renderInfo)
		: vao(renderInfo.vao),
		indexCount(renderInfo.indexCount),
		isColorOverride(renderInfo.isColorOverride),
		colorOverride(renderInfo.colorOverride),
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
		projViewModel(glm::mat4(1.0f))
	{}

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Render"))
		{
			// Diffuse Color
			if (isColorOverride)
			{
				ImGui::ColorEdit3("Color", (float*) &colorOverride);
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
	std::vector<std::pair<Texture*, float>> albedoTextures;

	// Materials Textures
	Texture* normalTexture;
	Texture* roughnessTexture;
	Texture* metalTexture;
	Texture* aoTexture;

	// Material values (should only be used if material textures are not present)
	float roughness;
	float metalness;
	float ao;

	bool isWireframe;
	bool isIgnoreLighting;

	float alphaTransparency;

	bool hasPrevProjViewModel;
	glm::mat4 projViewModel;
};