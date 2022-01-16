#include "ScenePanel.h"

#include "Renderer.h"

#include <unordered_map>
#include <string>
#include <iostream>

ScenePanel::ScenePanel()
{

}

ScenePanel::~ScenePanel()
{

}

void ScenePanel::OnUpdate()
{
    ImGui::Begin("Scene Editor");

    if (ImGui::CollapsingHeader("Lights"))
    {
        std::unordered_map<std::string, Light*> lightMap = Renderer::GetLights();
        std::unordered_map<std::string, Light*>::iterator it = lightMap.begin();
        while (it != lightMap.end())
        {
            Light* light = it->second;
            if (ImGui::TreeNode(it->first.c_str()))
            {
                ImGui::DragFloat3("Position", (float*) &light->position, 0.01f);
                if (light->GetLightType() == LightType::Directional)
                {
                    ImGui::DragFloat3("Direction", (float*)&light->direction, 0.01f);
                }
                ImGui::ColorEdit3("Diffuse", (float*) &light->color);
                ImGui::DragFloat("Radius", &light->radius.radius, 0.01f);
                if (light->GetLightType() == LightType::Point)
                {
                    ImGui::DragFloat("Intensity", &light->intensity, 0.01f);
                }

                ImGui::NewLine();

                // Atten selection
                if (light->lightType != LightType::Directional)
                {
                    ImGui::Text("Attenuation Mode");
                    if (ImGui::RadioButton("Linear", (int*) &light->attenuationMode, 0))
                    {
                        light->attenuationMode = AttenuationMode::Linear;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Quadratic", (int*)&light->attenuationMode, 1))
                    {
                        light->attenuationMode = AttenuationMode::Quadratic;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("UE4", (int*)&light->attenuationMode, 2))
                    {
                        light->attenuationMode = AttenuationMode::UE4;
                    }
                }
                
                ImGui::NewLine();

                // Light Type selection
                ImGui::Text("Light Type");
                if (ImGui::RadioButton("Directional", (int*) &light->lightType, 0))
                {
                    light->lightType = LightType::Directional;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Point", (int*) &light->lightType, 1))
                {
                    light->lightType = LightType::Point;
                }

                ImGui::NewLine();
                ImGui::Checkbox("On", &light->on);

                ImGui::TreePop();
            }

            light->SendToShader();

            it++;
        }
    }

    if (ImGui::CollapsingHeader("Deffered Geometry"))
    {
        std::unordered_map<std::string, SubmittedGeometry>& geometryMap = Renderer::GetDefferedGeometry();
        std::unordered_map<std::string, SubmittedGeometry>::iterator it = geometryMap.begin();
        while (it != geometryMap.end())
        {
            SubmittedGeometry& geometry = it->second;
            if (ImGui::TreeNode(it->first.c_str()))
            {
                ImGui::DragFloat3("Position", (float*)geometry.position, 0.01f);
                ImGui::DragFloat3("Scale", (float*)geometry.scale, 0.01f);
                ImGui::DragFloat3("Orientation", (float*)geometry.orientation, 0.01f);

                ImGui::NewLine();

                // TODO: Albedo texture stuff 
                
                ImGui::NewLine();

                // Material sliders
                if (!geometry.HasMaterialTextures()) // Material sliders
                {
                    ImGui::DragFloat("Roughness", &geometry.roughness, 0.001f, 0.0f, 1.0f);
                    ImGui::DragFloat("Metalness", &geometry.metalness, 0.001f, 0.0f, 10.0f);
                    ImGui::DragFloat("Ambient Occlusion", &geometry.ao, 0.001f, 0.0f, 1.0f);
                }
                else // Material texture editor
                {
                    // TODO
                }
             
                ImGui::TreePop();
            }

            it++;
        }
    }

    ImGui::End();
}