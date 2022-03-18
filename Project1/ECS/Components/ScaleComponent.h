#pragma once

#include "Component.h"

struct ScaleComponent : public Component
{
	ScaleComponent(glm::vec3 value) : value(value) {}
	ScaleComponent() : value(glm::vec3(1.0f)) {}

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