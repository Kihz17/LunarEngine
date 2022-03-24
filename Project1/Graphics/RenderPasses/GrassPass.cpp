#include "GrassPass.h"
#include "ShaderLibrary.h"
#include "TextureManager.h"
#include "Utils.h"

#include <vendor/imgui/imgui.h>

const std::string GrassPass::GRASS_SHADER_KEY = "grassShader";

GrassPass::GrassPass(int maxGrassBlades)
	: shader(ShaderLibrary::Load(GRASS_SHADER_KEY, "assets/shaders/grass.glsl")),
	grassVAO(new VertexArrayObject()),
	grassVBO(new VertexBuffer(maxGrassBlades * sizeof(glm::vec4))),
	maxGrassBlades(maxGrassBlades),
	oscillationStrength(2.5f),
	windForceMult(1.0f),
	stiffness(0.8f),
	windDirection(glm::vec2(0.1f, 0.9f)),
	grassDiscard(TextureManager::CreateTexture2D("assets/textures/grassBladeAlpha.png", TextureFilterType::Linear, TextureWrapType::ClampToEdge)),
	grassColor(TextureManager::CreateTexture2D("assets/textures/grassColor.png", TextureFilterType::Linear, TextureWrapType::Repeat))
{
	BufferLayout bufferLayout = {
		{ ShaderDataType::Float4, "vWorldPosition" }
	};

	grassVBO->SetLayout(bufferLayout);
	grassVAO->AddVertexBuffer(grassVBO);

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
}

GrassPass::~GrassPass()
{
	delete grassVAO;
}

void GrassPass::DoPass(IFrameBuffer* geometryBuffer, GrassCluster& grassCluster, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view)
{
	ImGui::Begin("Grass");
	if (ImGui::TreeNode("Grass Stuff"))
	{
		ImGui::DragFloat2("Wind Direction", (float*)&windDirection, 0.01f);
		ImGui::DragFloat("Wind Force", &windForceMult, 0.01f);
		ImGui::DragFloat("Oscillation Strength", &oscillationStrength, 0.01f);
		ImGui::DragFloat("Grass Stiffness", &stiffness, 0.01f);
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


	shader->SetFloat3("uWindParams", glm::vec3(grassCluster.oscillationStrength, grassCluster.windForceMult, grassCluster.stiffness));
	shader->SetFloat2("uWindDirection", glm::normalize(grassCluster.windDirection));
	shader->SetFloat2("uWidthHeight", grassCluster.dimensions);
	shader->SetInt("uHasNormalTexture", false);

	grassCluster.discardTexture->BindToSlot(0);
	shader->SetInt("uDiscardTexture", 0);

	grassCluster.albedoTexture->BindToSlot(1);
	shader->SetInt("uAlbedoTexture", 1);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	grassCluster.VAO->Bind();
	glDrawArrays(GL_POINTS, 0, grassCluster.grassData.size());
	grassCluster.VAO->Unbind();

	geometryBuffer->Bind();

	glDisable(GL_MULTISAMPLE);
}

void GrassPass::AddGrass(const std::vector<glm::vec4>& v)
{
	grassPositions.insert(grassPositions.end(), v.begin(), v.end());
	grassVBO->SetData(grassPositions.data(), maxGrassBlades * sizeof(glm::vec4));
}