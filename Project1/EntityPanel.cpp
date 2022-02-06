#include "EntityPanel.h"
#include "Components.h"
#include "Entity.h"

#include <unordered_map>
#include <string>
#include <iostream>

EntityPanel::EntityPanel()
{

}

EntityPanel::~EntityPanel()
{

}

void ShowEntity(const Entity* entity)
{
    if (ImGui::TreeNode(entity->name.c_str()))
    {
        ImGui::Text("Components:");
        for (Component* component : entity->GetComponents()) component->ImGuiUpdate();

        ImGui::NewLine();
        ImGui::Text("Children:");
        for (Entity* child : entity->GetChildren()) ShowEntity(child);

        ImGui::TreePop();
    }
}

void EntityPanel::Update(const std::vector<Entity*>& entities)
{
    ImGui::Begin("Entities");

    if (ImGui::CollapsingHeader("Entities"))
    {
        for(Entity* entity : entities)
        {
            ShowEntity(entity);
        }
    }

    ImGui::End();
}