#include "TerrainPass.h"
#include "TextureManager.h"
#include "ShaderLibrary.h"

const std::string TerrainPass::TERRAIN_SHADER_KEY = "terrainShader";

constexpr int numPatches = 4;

TerrainPass::TerrainPass()
    : heightMap(TextureManager::CreateTexture2D("assets/textures/heightmap.png", TextureFilterType::Linear, TextureWrapType::ClampToEdge)),
    vao(new VertexArrayObject()),
    shader(ShaderLibrary::Load(TERRAIN_SHADER_KEY, "assets/shaders/terrain.glsl"))
{
    std::vector<float> vertices;
    int width = heightMap->GetWidth();
    int height = heightMap->GetHeight();
    unsigned int res = 20;

    for (unsigned int i = 0; i <= res - 1; i++)
    {
        for (unsigned int j = 0; j <= res - 1; j++)
        {
            vertices.push_back(-width / 2.0f + width * i / (float)res);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * j / (float)res);
            vertices.push_back(i / (float)res);
            vertices.push_back(j / (float)res);

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)res);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * j / (float)res);
            vertices.push_back((i + 1) / (float)res);
            vertices.push_back(j / (float)res);

            vertices.push_back(-width / 2.0f + width * i / (float)res);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)res);
            vertices.push_back(i / (float)res);
            vertices.push_back((j + 1) / (float)res);

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)res);
            vertices.push_back(0);
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)res);
            vertices.push_back((i + 1) / (float)res);
            vertices.push_back((j + 1) / (float)res);
        }
    }

    vbo = new VertexBuffer(&vertices[0], vertices.size() * sizeof(float));
    BufferLayout layout = {
        {ShaderDataType::Float3, "vPosition"},
        {ShaderDataType::Float2, "vTextureCoordinates"}
    };
    vbo->SetLayout(layout);
    vao->AddVertexBuffer(vbo);
}

void TerrainPass::DoPass(const glm::mat4& projection, const glm::mat4& view)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glPatchParameteri(GL_PATCH_VERTICES, numPatches);

    shader->Bind();

    shader->SetMat4("uProjection", projection);
    shader->SetMat4("uView", view);

    vao->Bind();
    glDrawArrays(GL_PATCHES, 0, numPatches * 20 * 20);
}