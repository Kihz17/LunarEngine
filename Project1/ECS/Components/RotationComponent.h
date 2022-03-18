#pragma once

#include "Component.h"

#include <glm/gtx/quaternion.hpp>

struct RotationComponent : public Component
{
	RotationComponent(const glm::quat& quat) : value(quat) {}
	RotationComponent() : value(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {}

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