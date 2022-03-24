#include "TerrainPass.h"
#include "TextureManager.h"
#include "ShaderLibrary.h"
#include "Utils.h"

#include "vendor/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

const std::string TerrainPass::TERRAIN_SHADER_KEY = "terrainShader";

TerrainPass::TerrainPass()
    : vao(new VertexArrayObject()),
    shader(ShaderLibrary::Load(TERRAIN_SHADER_KEY, "assets/shaders/terrain.glsl")),
    patchCount(20), // Represents the number of patches we will divide the terrain into
    numPatchPrimitives(4),
    terrainTexture(TextureManager::CreateTexture2D("assets/textures/terrain.jpg", TextureFilterType::Linear, TextureWrapType::Repeat)),
    terrainTextureScale(500.0f)
{
    std::vector<float> vertices;

    // Matches far plane
    constexpr int width = 1000;
    constexpr int height = 1000;

    for (unsigned int i = 0; i < patchCount; i++)
    {
        for (unsigned int j = 0; j < patchCount; j++)
        {
            vertices.push_back(-width / 2.0f + width * i / (float)patchCount);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * j / (float)patchCount);
            vertices.push_back(i / (float)patchCount);
            vertices.push_back(j / (float)patchCount);

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)patchCount);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * j / (float)patchCount);
            vertices.push_back((i + 1) / (float)patchCount);
            vertices.push_back(j / (float)patchCount);

            vertices.push_back(-width / 2.0f + width * i / (float)patchCount);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)patchCount);
            vertices.push_back(i / (float)patchCount);
            vertices.push_back((j + 1) / (float)patchCount);

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)patchCount);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)patchCount);
            vertices.push_back((i + 1) / (float)patchCount);
            vertices.push_back((j + 1) / (float)patchCount);
        }
    }

    vbo = new VertexBuffer(vertices.data(), vertices.size() * sizeof(float));
    BufferLayout layout = {
        {ShaderDataType::Float3, "vPosition"},
        {ShaderDataType::Float2, "vTextureCoordinates"}
    };

    vbo->SetLayout(layout);
    vao->AddVertexBuffer(vbo);

    shader->InitializeUniform("uProjection");
    shader->InitializeUniform("uModel");
    shader->InitializeUniform("uView");
    shader->InitializeUniform("uSeed");
    shader->InitializeUniform("uCameraPosition");
    shader->InitializeUniform("uTerrainParams");
    shader->InitializeUniform("uOctaves");
    shader->InitializeUniform("uTerrainTexture");
    shader->InitializeUniform("uTextureCoordScale");
}

void TerrainPass::DoPass(IFrameBuffer* geometryBuffer, TerrainGenerationInfo& terrainInfo, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos)
{
    ImGui::Begin("Terrain");
    if (ImGui::TreeNode("Terrain Stuff"))
    {
        ImGui::DragFloat("Amplitude", &terrainInfo.amplitude, 0.01f);
        ImGui::DragFloat("Roughness", &terrainInfo.roughness, 0.001f);
        ImGui::DragFloat("Frequency", &terrainInfo.frequency, 0.0001f, 0.0f, 5.0f, "%.4f");
        ImGui::DragFloat("Persistence", &terrainInfo.persitence, 0.001f);
        ImGui::DragInt("Octaves", &terrainInfo.octaves);
        ImGui::DragFloat("Texture Scale", &terrainTextureScale, 0.1f);
        ImGui::TreePop();
    }
    ImGui::End();

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glPatchParameteri(GL_PATCH_VERTICES, numPatchPrimitives); // Consider every n vertices to be a "patch primitive"

    geometryBuffer->Bind();

    shader->Bind();

    glm::vec2 shaderPos(cameraPos.x, cameraPos.z);
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraPos.x, 0.0f, cameraPos.z));

    shader->SetMat4("uProjection", projection);
    shader->SetMat4("uView", view);
    shader->SetMat4("uModel", transform);
    shader->SetFloat2("uCameraPosition", shaderPos);
    shader->SetFloat2("uSeed", terrainInfo.seed);
    shader->SetFloat4("uTerrainParams", glm::vec4(terrainInfo.amplitude, terrainInfo.roughness, terrainInfo.persitence, terrainInfo.frequency));
    shader->SetInt("uOctaves", terrainInfo.octaves);
    shader->SetFloat("uTextureCoordScale", terrainTextureScale);

    terrainTexture->BindToSlot(0);
    shader->SetInt("uTerrainTexture", 0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    vao->Bind();
    glDrawArrays(GL_PATCHES, 0, numPatchPrimitives * patchCount * patchCount);

    geometryBuffer->Unbind();
}