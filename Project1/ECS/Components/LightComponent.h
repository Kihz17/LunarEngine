#pragma once

#include "Component.h"
#include "Light.h"

struct LightComponent : public Component
{
	LightComponent() : ptr(nullptr) {}
	LightComponent(Light* ptr) : ptr(ptr) {}
	virtual ~LightComponent()
	{
		delete ptr;
	}

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Light"))
		{
			ImGui::DragFloat3("Position", (float*)&ptr->position, 0.01f);
			if (ptr->lightType == LightType::Directional)
			{
				ImGui::DragFloat3("Direction", (float*)&ptr->direction, 0.01f);
			}
			ImGui::ColorEdit3("Diffuse", (float*)&ptr->color);
			ImGui::DragFloat("Radius", &ptr->radius, 0.01f);
			ImGui::DragFloat("Intensity", &ptr->intensity, 0.01f);
			
			ImGui::NewLine();

			// Atten selection
			if (ptr->lightType != LightType::Directional)
			{
				ImGui::Text("Attenuation Mode");
				if (ImGui::RadioButton("Linear", (int*)&ptr->attenuationMode, 0))
				{
					ptr->attenuationMode = AttenuationMode::Linear;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Quadratic", (int*)&ptr->attenuationMode, 1))
				{
					ptr->attenuationMode = AttenuationMode::Quadratic;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("UE4", (int*)&ptr->attenuationMode, 2))
				{
					ptr->attenuationMode = AttenuationMode::UE4;
				}
			}

			ImGui::NewLine();

			// Light Type selection
			ImGui::Text("Light Type");
			if (ImGui::RadioButton("Directional", (int*)&ptr->lightType, 0))
			{
				ptr->lightType = LightType::Directional;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Point", (int*)&ptr->lightType, 1))
			{
				ptr->lightType = LightType::Point;
			}

			ImGui::NewLine();
			ImGui::Checkbox("On", &ptr->on);

			ptr->SendToShader();

			ImGui::TreePop();
		}
	}

	Light* ptr;
};