#pragma once

#include "Component.h"

struct PositionComponent : public Component
{
	PositionComponent(glm::vec3 value) : value(value) {}
	PositionComponent() : value(glm::vec3(0.0f)) {}

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