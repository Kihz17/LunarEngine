#pragma once

#include "VertexArrayObject.h"
#include "Texture.h"
#include "Light.h"
#include "ISteeringCondition.h"

#include <IRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "vendor/imgui/imgui.h"

#include <unordered_set>
#include <string>

struct Component
{
	virtual ~Component() = default;

	virtual void ImGuiUpdate() = 0;

protected:
	Component() = default;
};

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

struct VelocityComponent : public Component
{
	VelocityComponent() : value(glm::vec3(0.0f)) {}

	virtual void ImGuiUpdate() override
	{
	
	}

	glm::vec3 value;
};

struct RigidBodyComponent : public Component
{
	RigidBodyComponent() : ptr(nullptr) {}
	RigidBodyComponent(Physics::IRigidBody* body) : ptr(body) {}

	virtual void ImGuiUpdate() override
	{

	}

	Physics::IRigidBody* ptr;
};

struct RenderComponent : public Component
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
		projViewModel(glm::mat4(1.0f))
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

struct LightComponent : public Component
{
	LightComponent() : ptr(nullptr) {}
	LightComponent(Light* ptr) : ptr(ptr) {}

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
			if (ptr->lightType == LightType::Point)
			{
				ImGui::DragFloat("Intensity", &ptr->intensity, 0.01f);
			}

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

enum EasingType
{
	EaseIn,
	EaseOut,
	EaseInOut,
	None
};

#define DIRECT_ROTATION 0
#define LERP_ROTATION 1
#define SLERP_ROTATION 2

struct KeyFramePositionComponent : Component
{
	KeyFramePositionComponent() = default;
	KeyFramePositionComponent(float time, const glm::vec3& position, EasingType easingType = EasingType::None)
		: time(time), 
		position(position), 
		easingType(easingType) 
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::vec3 position;
	EasingType easingType;
};

struct KeyFrameScaleComponent : Component
{
	KeyFrameScaleComponent() = default;
	KeyFrameScaleComponent(float time, const glm::vec3& scale, EasingType easingType = EasingType::None)
		: time(time), 
		scale(scale), 
		easingType(easingType) 
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::vec3 scale;
	EasingType easingType;
};

struct KeyFrameRotationComponent : Component
{
	KeyFrameRotationComponent() = default;
	KeyFrameRotationComponent(float time, const glm::quat& rotation, int interpolationType = 0, EasingType easingType = EasingType::None)
		: time(time), 
		rotation(rotation), 
		interpolationType(interpolationType), 
		easingType(easingType) 
	{}

	virtual void ImGuiUpdate() override
	{

	}

	float time;
	glm::quat rotation;
	EasingType easingType;
	int interpolationType;
};

struct AnimationComponent : Component
{
	AnimationComponent() = default;

	virtual void ImGuiUpdate() override
	{
		if (ImGui::TreeNode("Animation"))
		{
			ImGui::DragFloat("Speed", &speed, 0.01);

			ImGui::NewLine();
			ImGui::Checkbox("Playing", &playing);

			ImGui::TreePop();
		}
	}

	bool playing;
	float duration;
	float currentTime;
	float speed;
	bool repeat;
	std::vector<KeyFramePositionComponent> keyFramePositions;
	std::vector<KeyFrameScaleComponent> keyFrameScales;
	std::vector<KeyFrameRotationComponent> keyFrameRotations;
};

struct SteeringBehaviourComponent : public Component
{
	SteeringBehaviourComponent(std::vector<ISteeringCondition*> behaviours) : behaviours(behaviours) {}
	SteeringBehaviourComponent() {}

	virtual void ImGuiUpdate() override
	{

	}

	void AddBehaviour(int priority, ISteeringCondition* behaviour)
	{
		if (behaviour->GetBehaviour()->GetType() != SteeringBehaviourType::Normal) return;

		int prioSize = priority + 1;
		if (prioSize > behaviours.size())
		{
			behaviours.resize(prioSize);
		}

		behaviours[priority] = behaviour;
	}

	void AddTargetingBehaviour(int priority, ISteeringCondition* behaviour)
	{
		if (behaviour->GetBehaviour()->GetType() != SteeringBehaviourType::Targeting) return;

		int prioSize = priority + 1;
		if (prioSize > targetingBehaviours.size())
		{
			targetingBehaviours.resize(prioSize);
		}

		targetingBehaviours[priority] = behaviour;
	}

private:
	friend class AILayer;

	ISteeringCondition* active = nullptr;
	int activePriority = -1;

	std::vector<ISteeringCondition*> behaviours;
	std::vector<ISteeringCondition*> targetingBehaviours;
};

struct TagComponent : public Component
{
	TagComponent() {}

	void AddTag(const std::string& tag) { tags.insert(tag); }
	bool HasTag(const std::string& tag) const { return tags.find(tag) != tags.end(); }

	std::unordered_set<std::string> tags;
};