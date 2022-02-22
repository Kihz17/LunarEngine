#pragma once

#include "Model.h"
#include "Component.h"
#include "ITexture.h"
#include "ReflectRefract.h"

struct RenderComponent : public Component
{
	RenderComponent() : model(nullptr) {}
	RenderComponent(Model* model) : model(model) {}

	virtual void ImGuiUpdate() override
	{
		//if (ImGui::TreeNode("Render"))
		//{
		//	// Diffuse Color
		//	if (isColorOverride)
		//	{
		//		ImGui::ColorEdit3("Color", (float*)&colorOverride);
		//	}
		//	else
		//	{
		//		// TODO: Allow changing albedo textures and ratios
		//	}

		//	ImGui::NewLine();

		//	// Materials
		//	if (!HasMaterialTextures())
		//	{
		//		ImGui::DragFloat("Roughness", &roughness, 0.001f, 0.0f, 1.0f);
		//		ImGui::DragFloat("Metalness", &metalness, 0.001f, 0.0f, 1.0f);
		//		ImGui::DragFloat("Ambient Occlusion", &ao, 0.001f, 0.0f, 1.0f);
		//	}
		//	else
		//	{
		//		// TODO: Change material textures
		//	}

		//	ImGui::NewLine();

		//	ImGui::DragFloat("Transparency", &alphaTransparency, 0.001f, 0.0f, 1.0f);
		//	ImGui::Checkbox("Ignore Lighting", &isIgnoreLighting);
		//	ImGui::Checkbox("Cast Shadows", &castShadows);
		//	ImGui::Checkbox("Cast Shadows On", &castShadowsOn);
		//	ImGui::DragFloat("Shadow Softness", &shadowSoftness, 0.001f, 0.0f, 1.0f);
		//	
		//	ImGui::NewLine();
		//	ImGui::DragFloat("Reflect/Refract Strength", &reflectRefractData.strength, 0.001f, 0.0f, 1.0f);
		//	ImGui::DragFloat("Refract Ratio", &reflectRefractData.refractRatio, 0.001f);

		//	ImGui::Text("Reflection/Refraction Type:");
		//	int renderSelection = (int) reflectRefractData.type;
		//	ImGui::RadioButton("None", &renderSelection, 0); ImGui::SameLine();
		//	ImGui::RadioButton("Reflect", &renderSelection, 1); ImGui::SameLine();
		//	ImGui::RadioButton("Refract", &renderSelection, 2);

		//	if (renderSelection != (int)reflectRefractData.type)
		//	{
		//		reflectRefractData.type = (ReflectRefractType) renderSelection;
		//	}

		//	ImGui::NewLine();
		//	ImGui::Text("Reflection/Refraction Map Type:");
		//	int rrMapTypeSelection = (int) reflectRefractData.mapType;
		//	ImGui::RadioButton("Environment", &rrMapTypeSelection, 0);
		//	ImGui::RadioButton("Dynamic Minimal", &rrMapTypeSelection, 1);
		//	ImGui::RadioButton("Dynamic Medium", &rrMapTypeSelection, 2); 
		//	ImGui::RadioButton("Dynamic Full", &rrMapTypeSelection, 3);

		//	if (rrMapTypeSelection != (int)reflectRefractData.mapType)
		//	{
		//		reflectRefractData.mapType = (ReflectRefractMapType) rrMapTypeSelection;
		//	}

		//	ImGui::TreePop();
		//}

	}

	Model* model;
};