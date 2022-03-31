#include "EditorLayer.h"
#include "RenderComponent.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "EntitySerializer.h"
#include "vendor/imgui/imgui.h"
#include "MeshManager.h"
#include "Renderer.h"
#include "GrassSerializer.h"

#include "vendor/imguizmo/imGuIZMOquat.h"

#include <fstream>
#include <iostream>

static const char* components[7] =
{
	"Animation",
	"Light",
	"Position",
	"Render",
	"Rigidbody",
	"Rotation",
	"Scale"
};

static const char* textureFilters[3] =
{
	"None",
	"Linear",
	"Nearest"
};

static const char* textureWraps[4] =
{
	"None",
	"ClampToEdge",
	"ClamptoBorder",
	"Repeat"
};

static const char* divider = "===========================================";

EditorLayer::EditorLayer(EntityManager& entityManager, Physics::IPhysicsWorld<Entity>* physworld)
	: entityManager(entityManager),
	physWorld(physworld),
	currentComponent(0),
	currentTextureFilter(0),
	currentTextureWrap(0),
	currentMatTextureFilter(0),
	currentMatTextureWrap(0)
{

}

void EditorLayer::OnUpdate(float deltaTime)
{
	{
		static char entityName[30] = "";

		ImGui::Begin("Editor");
		if (ImGui::Button("Add Entity"))
		{
			std::string name(entityName);
			if (name.empty())
			{
				entityManager.CreateEntity();
			}
			else
			{
				entityManager.CreateEntity(name);
			}
		}

		ImGui::InputText("Entity Name", entityName, IM_ARRAYSIZE(entityName));

		// Save scene
		ImGui::NewLine();
		static char savePath[100] = "";
		ImGui::InputText("Save Path", savePath, IM_ARRAYSIZE(savePath));
		if (ImGui::Button("Save Scene") && !std::string(savePath).empty())
		{
			YAML::Emitter out;

			out << YAML::BeginMap;
			out << YAML::Key << "Scene" << YAML::Value << "Untitled";

			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			for (Entity* e : entityManager.GetEntities())
			{
				if (!e->shouldSave) continue;
				EntitySerializer(e, entityManager).Serialize(out);
			}
			out << YAML::EndSeq;

			out << YAML::Key << "Grass" << YAML::Value << YAML::BeginSeq;
			for (GrassCluster& grass : Renderer::GetGrassClusters())
			{
				GrassSerializer(grass).Serialize(out);
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;

			std::ofstream ofs(savePath);
			ofs << out.c_str();
		}

		ImGui::End();
	}
	
	{
		ImGui::Begin("Entities");

		if (ImGui::CollapsingHeader("Entities"))
		{
			for (Entity* entity : entityManager.GetEntities())
			{
				if (!entity->shouldSave) continue;
				ShowEntity(entity);
			}
		}

		ImGui::End();
	}
}

void EditorLayer::ShowEntity(Entity* entity)
{
	if (ImGui::TreeNode(entity->name.c_str()))
	{
		ImGui::Text("Components:");
		for (Component* component : entity->GetComponents()) ShowComponent(component);

		ImGui::NewLine();
		ImGui::Text("Children:");
		for (Entity* child : entity->GetChildren()) ShowEntity(child);

		ImGui::NewLine();
		ImGui::Text("Add Component:");
		ImGui::Combo(("Components##" + entity->name).c_str(), &currentComponent, components, IM_ARRAYSIZE(components));
		if (ImGui::Button(("Add Component##" + entity->name).c_str()))
		{
			if (currentComponent == 0 && !entity->HasComponent<SkeletalAnimationComponent>()) // animation
			{
				entity->AddComponent<SkeletalAnimationComponent>();
			}
			else if (currentComponent == 1 && !entity->HasComponent<LightComponent>()) // light
			{
				entity->AddComponent<LightComponent>();
			}
			else if (currentComponent == 2 && !entity->HasComponent<PositionComponent>()) // position
			{
				entity->AddComponent<PositionComponent>();
			}
			else if (currentComponent == 3 && !entity->HasComponent<RenderComponent>()) // render
			{
				entity->AddComponent<RenderComponent>();
			}
			else if (currentComponent == 4 && !entity->HasComponent<RigidBodyComponent>()) // rigidbody
			{
				entity->AddComponent<RigidBodyComponent>();
			}
			else if (currentComponent == 5 && !entity->HasComponent<RotationComponent>()) // rotation
			{
				entity->AddComponent<RotationComponent>();
			}
			else if (currentComponent == 6 && !entity->HasComponent<ScaleComponent>()) // scale
			{
				entity->AddComponent<ScaleComponent>();
			}
		}

		ImGui::TreePop();
	}
}

void EditorLayer::ShowComponent(Component* comp)
{
	if (dynamic_cast<SkeletalAnimationComponent*>(comp))
	{
		SkeletalAnimationComponent* c = dynamic_cast<SkeletalAnimationComponent*>(comp);
	}
	else if (dynamic_cast<LightComponent*>(comp))
	{
		LightComponent* c = dynamic_cast<LightComponent*>(comp);
		if (ImGui::TreeNode("Light"))
		{
			ImGui::DragFloat3("Position", (float*)&c->ptr->position, 0.01f);
			if (c->ptr->lightType == LightType::Directional)
			{
				ImGui::DragFloat3("Direction", (float*)&c->ptr->direction, 0.01f);
			}
			ImGui::ColorEdit3("Diffuse", (float*)&c->ptr->color);
			ImGui::DragFloat("Radius", &c->ptr->radius, 0.01f);
			ImGui::DragFloat("Intensity", &c->ptr->intensity, 0.01f);

			ImGui::NewLine();

			// Atten selection
			if (c->ptr->lightType != LightType::Directional)
			{
				ImGui::Text("Attenuation Mode");
				if (ImGui::RadioButton("Linear", (int*)&c->ptr->attenuationMode, 0))
				{
					c->ptr->attenuationMode = AttenuationMode::Linear;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Quadratic", (int*)&c->ptr->attenuationMode, 1))
				{
					c->ptr->attenuationMode = AttenuationMode::Quadratic;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("UE4", (int*)&c->ptr->attenuationMode, 2))
				{
					c->ptr->attenuationMode = AttenuationMode::UE4;
				}
			}

			ImGui::NewLine();

			// Light Type selection
			ImGui::Text("Light Type");
			if (ImGui::RadioButton("Directional", (int*)&c->ptr->lightType, 0))
			{
				c->ptr->lightType = LightType::Directional;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Point", (int*)&c->ptr->lightType, 1))
			{
				c->ptr->lightType = LightType::Point;
			}

			ImGui::NewLine();
			ImGui::Checkbox("On", &c->ptr->on);

			c->ptr->SendToShader();

			ImGui::TreePop();
		}
	}
	else if (dynamic_cast<PositionComponent*>(comp))
	{
		PositionComponent* c = dynamic_cast<PositionComponent*>(comp);
		if (ImGui::TreeNode("Position Component"))
		{
			ImGui::DragFloat3("Position", (float*)&c->value, 0.01f);
			ImGui::TreePop();
		}
	}
	else if (dynamic_cast<RenderComponent*>(comp))
	{
		RenderComponent* c = dynamic_cast<RenderComponent*>(comp);
		if (ImGui::TreeNode("Render"))
		{
			// Change mesh
			{
				static char meshPath[100] = "";
				ImGui::InputText("Mesh Path", meshPath, IM_ARRAYSIZE(meshPath));
				if (ImGui::Button("Set Mesh"))
				{
					Mesh* m = MeshManager::GetMesh("assets/models/" + std::string(meshPath));
					if(m) c->mesh = m;
				}
			}

			// Diffuse Color
			ImGui::Text(divider);
			ImGui::Checkbox("Color Override", &c->isColorOverride);
			if (c->isColorOverride)
			{
				ImGui::ColorEdit3("Color", (float*)&c->colorOverride);
			}
			else
			{
				// List textures
				{
					if(!c->albedoTextures.empty()) ImGui::Text("Albedo Textures:");
					int textureToRemove = -1;
					for (int i = 0; i < c->albedoTextures.size(); i++)
					{
						std::string si = std::to_string(i);
						ImGui::Text(("Texture" + si + ":").c_str());
						ImGui::DragFloat(("Strength##" + si).c_str(), &c->albedoTextures[i].second, 0.01f, 0.0f, 1.0f);
						ImGui::DragFloat2(("Offset##" + si).c_str(), (float*)&c->uvOffset, 0.01f);
						if (ImGui::Button(("Remove Texture##" + si).c_str()))
						{
							textureToRemove = i;
						}
						ImGui::NewLine();
					}

					if (textureToRemove != -1) // Remove texture if we should
					{
						c->albedoTextures.erase(c->albedoTextures.begin() + textureToRemove);
					}
				}
			
				ImGui::NewLine();
				ImGui::Text("Add Albedo Texture:");

				// Add Texture
				{
					static char texPath[100] = "";
					static bool genMips = true;
					ImGui::Combo("Albedo Filter##Albedo", &currentTextureFilter, textureFilters, IM_ARRAYSIZE(textureFilters));
					ImGui::Combo("Albedo Wrap##Albedo", &currentTextureWrap, textureWraps, IM_ARRAYSIZE(textureWraps));
					ImGui::Checkbox("Gen Mip Maps##Albedo", &genMips);
					ImGui::InputText("Texture Path##Albedo", texPath, IM_ARRAYSIZE(texPath));
					if (ImGui::Button("Add Albedo") && c->albedoTextures.size() < 4)
					{
						Texture2D* t = TextureManager::CreateTexture2D("assets/textures/" + std::string(texPath), (TextureFilterType)currentTextureFilter, (TextureWrapType)currentTextureWrap, genMips);
						if (t)
						{
							c->albedoTextures.push_back({ t, 1.0f });
						}
						else
						{
							std::cout << "Couldn't add texture " << texPath << ".\n";
						}
					}
				}	
			}
		
			ImGui::Text(divider);

			// Materials
			{
				if (!c->HasMaterialTextures())
				{
					ImGui::DragFloat("Roughness", &c->roughness, 0.001f, 0.0f, 1.0f);
					ImGui::DragFloat("Metalness", &c->metalness, 0.001f, 0.0f, 1.0f);
					ImGui::DragFloat("Ambient Occlusion", &c->ao, 0.001f, 0.0f, 1.0f);
				}
				else
				{
					if (ImGui::Button("Remove ORM Map")) c->ormTexture = nullptr;
				}

				if (c->normalTexture)
				{
					if (ImGui::Button("Remove Normal Map")) c->normalTexture = nullptr;
				}

				ImGui::NewLine();

				static char texPath[100] = "";
				static bool genMips = true;
				ImGui::Combo("Albedo Filter##Mat", &currentMatTextureFilter, textureFilters, IM_ARRAYSIZE(textureFilters));
				ImGui::Combo("Albedo Wrap##Mat", &currentMatTextureWrap, textureWraps, IM_ARRAYSIZE(textureWraps));
				ImGui::Checkbox("Gen Mip Maps##Mat", &genMips);
				ImGui::InputText("Texture Path##Mat", texPath, IM_ARRAYSIZE(texPath));

				if (ImGui::Button("Set Normal Map"))
				{
					Texture2D* t = TextureManager::CreateTexture2D("assets/textures/" + std::string(texPath), (TextureFilterType)currentMatTextureFilter, (TextureWrapType)currentMatTextureWrap, genMips);
					if (t)
					{
						c->normalTexture = t;
					}
					else
					{
						std::cout << "Couldn't set Normal map " << texPath << ".\n";
					}
				}

				if (ImGui::Button("Set ORM Map"))
				{
					Texture2D* t = TextureManager::CreateTexture2D("assets/textures/" + std::string(texPath), (TextureFilterType)currentMatTextureFilter, (TextureWrapType)currentMatTextureWrap, genMips);
					if (t)
					{
						c->ormTexture = t;
					}
					else
					{
						std::cout << "Couldn't set ORM map " << texPath << ".\n";
					}
				}
			}
			
			ImGui::Text(divider);

			ImGui::DragFloat("Transparency", &c->alphaTransparency, 0.001f, 0.0f, 1.0f);
			ImGui::Checkbox("Ignore Lighting", &c->isIgnoreLighting);
			ImGui::Checkbox("Cast Shadows", &c->castShadows);
			ImGui::Checkbox("Cast Shadows On", &c->castShadowsOn);
			ImGui::DragFloat("Surface Shadow Softness", &c->surfaceShadowSoftness, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Casting Shadow Softness", &c->castingShadownSoftness, 0.001f, 0.0f, 1.0f);

			ImGui::NewLine();
			ImGui::DragFloat("Reflect/Refract Strength", &c->reflectRefractData.strength, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Refract Ratio", &c->reflectRefractData.refractRatio, 0.001f);

			ImGui::Text("Reflection/Refraction Type:");
			int renderSelection = (int)c->reflectRefractData.type;
			ImGui::RadioButton("None", &renderSelection, 0); ImGui::SameLine();
			ImGui::RadioButton("Reflect", &renderSelection, 1); ImGui::SameLine();
			ImGui::RadioButton("Refract", &renderSelection, 2);

			if (renderSelection != (int)c->reflectRefractData.type)
			{
				c->reflectRefractData.type = (ReflectRefractType)renderSelection;
			}

			ImGui::NewLine();
			ImGui::Text("Reflection/Refraction Map Type:");
			int rrMapTypeSelection = (int)c->reflectRefractData.mapType;
			ImGui::RadioButton("Environment", &rrMapTypeSelection, 0);
			ImGui::RadioButton("Dynamic Minimal", &rrMapTypeSelection, 1);
			ImGui::RadioButton("Dynamic Medium", &rrMapTypeSelection, 2);
			ImGui::RadioButton("Dynamic Full", &rrMapTypeSelection, 3);

			if (rrMapTypeSelection != (int)c->reflectRefractData.mapType)
			{
				c->reflectRefractData.mapType = (ReflectRefractMapType)rrMapTypeSelection;
			}

			ImGui::TreePop();
		}
	}
	else if (dynamic_cast<RigidBodyComponent*>(comp))
	{
		RigidBodyComponent* c = dynamic_cast<RigidBodyComponent*>(comp);
		if (ImGui::TreeNode("Rigidbody"))
		{
			if (c->ptr)
			{
				/*if (ImGui::Button("Remove Body"))
				{
					physWorld->RemoveRigidBody(c->ptr);
					delete c->ptr;
					c->ptr = nullptr;
				}*/

				glm::vec3 position = c->ptr->GetPosition();
				glm::vec3 initialPos = position;
				ImGui::DragFloat3("Position",(float*) &position, 0.01f);
				if(position != initialPos) c->ptr->SetPosition(position);

				glm::quat rot = c->ptr->GetOrientation();
				glm::quat initialRot = rot;
				ImGui::DragFloat4("Rotation", (float*)&rot, 0.01f);
				if(rot != initialRot) c->ptr->SetOrientation(rot);
			}
			else
			{

			}

			ImGui::TreePop();
		}
	}
	else if (dynamic_cast<RotationComponent*>(comp))
	{
		RotationComponent* c = dynamic_cast<RotationComponent*>(comp);
		if (ImGui::TreeNode("Rotation"))
		{
			ImGui::DragFloat4("Rotation", (float*)&c->value, 0.01f);
			quat imguiQuat;
			imguiQuat.w = c->value.w;
			imguiQuat.x = c->value.x;
			imguiQuat.y = c->value.y;
			imguiQuat.z = c->value.z;
			ImGui::gizmo3D("Rot", imguiQuat, 100.0f, 3);
		
			c->value = glm::quat(imguiQuat.w, imguiQuat.x, imguiQuat.y, imguiQuat.z);
			ImGui::TreePop();
		}
	}
	else if (dynamic_cast<ScaleComponent*>(comp))
	{
		ScaleComponent* c = dynamic_cast<ScaleComponent*>(comp);
		if (ImGui::TreeNode("Scale"))
		{
			ImGui::DragFloat3("Scale", (float*)&c->value, 0.01f);
			ImGui::TreePop();
		}
	}
}