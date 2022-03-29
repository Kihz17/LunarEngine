#include "ProceduralGrassPass.h"
#include "ShaderLibrary.h"
#include "TextureManager.h"
#include "Utils.h"

#include <vendor/imgui/imgui.h>

const std::string ProceduralGrassPass::GRASS_SHADER_KEY = "proceduralGrassShader";

ProceduralGrassPass::ProceduralGrassPass()
	: shader(ShaderLibrary::Load(GRASS_SHADER_KEY, "assets/shaders/proceduralGrass.glsl"))
{
	shader->InitializeUniform("uWidthHeight");
	shader->InitializeUniform("uLODLevel");
	shader->InitializeUniform("uAlbedoTexture");
	shader->InitializeUniform("uHasNormalTexture");
	shader->InitializeUniform("uNormalTexture");
	shader->InitializeUniform("uRoughnessTexture");
	shader->InitializeUniform("uMetalnessTexture");
	shader->InitializeUniform("uAmbientOcculsionTexture");
	shader->InitializeUniform("uMaterialOverrides");
	shader->InitializeUniform("uDiscardTexture");
	shader->InitializeUniform("uCameraPosition");
	shader->InitializeUniform("uProjection");
	shader->InitializeUniform("uView");
	shader->InitializeUniform("uTime");
	shader->InitializeUniform("uWindParams");
	shader->InitializeUniform("uWindDirection");
	shader->InitializeUniform("uSeed");
	shader->InitializeUniform("uTerrainParams");
	shader->InitializeUniform("uOctaves");
}

ProceduralGrassPass::~ProceduralGrassPass()
{
	
}

void ProceduralGrassPass::DoPass(IFrameBuffer* geometryBuffer, GrassCluster& cluster, const TerrainGenerationInfo& terrainInfo, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view)
{
	ImGui::Begin("Grass");
	if (ImGui::TreeNode("Grass Stuff"))
	{
		ImGui::DragFloat2("Grass Dimensions", (float*)&cluster.dimensions, 0.01f);
		ImGui::DragFloat2("Wind Direction", (float*)&cluster.windDirection, 0.01f);
		ImGui::DragFloat("Wind Force", &cluster.windForceMult, 0.01f);
		ImGui::DragFloat("Oscillation Strength", &cluster.oscillationStrength, 0.01f);
		ImGui::DragFloat("Grass Stiffness", &cluster.stiffness, 0.01f);
		ImGui::TreePop();
	}
	ImGui::End();

	glDisable(GL_CULL_FACE); // Don't face cull, we want to render both sides of grass blades
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	geometryBuffer->Bind();

	shader->Bind();

	shader->SetFloat3("uCameraPosition", cameraPos);
	shader->SetMat4("uProjection", proj);
	shader->SetMat4("uView", view);
	shader->SetFloat("uTime", glfwGetTime());

	shader->SetFloat2("uSeed", terrainInfo.seed);
	shader->SetFloat4("uTerrainParams", glm::vec4(terrainInfo.amplitude, terrainInfo.roughness, terrainInfo.persitence, terrainInfo.frequency));
	shader->SetInt("uOctaves", terrainInfo.octaves);

	shader->SetFloat3("uWindParams", glm::vec3(cluster.oscillationStrength, cluster.windForceMult, cluster.stiffness));
	shader->SetFloat2("uWindDirection", glm::normalize(cluster.windDirection));
	shader->SetFloat2("uWidthHeight", cluster.dimensions);

	cluster.discardTexture->BindToSlot(0);
	shader->SetInt("uDiscardTexture", 0);

	cluster.albedoTexture->BindToSlot(1);
	shader->SetInt("uAlbedoTexture", 1);

	// TODO: Normal texture 
	shader->SetInt("uHasNormalTexture", false);

	shader->SetFloat4("uMaterialOverrides", glm::vec4(cluster.roughness, cluster.metalness, cluster.ao, 1.0f));
	

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	cluster.VAO->Bind();
	glDrawArrays(GL_POINTS, 0, cluster.grassData.size());
	cluster.VAO->Unbind();

	geometryBuffer->Bind();

	glDisable(GL_MULTISAMPLE);
}