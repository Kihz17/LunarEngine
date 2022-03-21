#include "GrassPass.h"
#include "ShaderLibrary.h"

#include <vendor/imgui/imgui.h>

const std::string GrassPass::GRASS_SHADER_KEY = "grassShader";

GrassPass::GrassPass(int maxGrassBlades)
	: shader(ShaderLibrary::Load(GRASS_SHADER_KEY, "assets/shaders/grass.glsl")),
	grassVAO(new VertexArrayObject()),
	grassVBO(new VertexBuffer(maxGrassBlades * sizeof(glm::vec3))),
	maxGrassBlades(maxGrassBlades),
	oscillationStrength(2.5f),
	windForceMult(1.0f),
	windDirection(glm::vec2(0.1f, 0.9f))
{
	BufferLayout bufferLayout = {
		{ ShaderDataType::Float3, "vWorldPosition" }
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

void GrassPass::DoPass(const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view)
{
	ImGui::Begin("Grass");
	if (ImGui::TreeNode("Grass Stuff"))
	{
		ImGui::DragFloat2("Wind Direction", (float*)&windDirection, 0.01f);
		ImGui::DragFloat("Wind Force", &windForceMult, 0.01f);
		ImGui::DragFloat("Oscillation Strength", &oscillationStrength, 0.01f);
		ImGui::TreePop();
	}
	ImGui::End();

	glDisable(GL_CULL_FACE); // Don't face cull, we want to render both sides of grass blades

	shader->Bind();
	shader->SetFloat3("uCameraPosition", cameraPos);
	shader->SetMat4("uProjection", proj);
	shader->SetMat4("uView", view);
	shader->SetFloat("uTime", glfwGetTime());
	shader->SetFloat2("uWindParams", glm::vec2(oscillationStrength, windForceMult));
	shader->SetFloat2("uWindDirection", glm::normalize(windDirection));
	shader->SetFloat2("uWidthHeight", glm::vec2(0.5f, 0.5f));

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	grassVAO->Bind();
	glDrawArrays(GL_POINTS, 0, grassPositions.size());
	grassVAO->Unbind();
}

void GrassPass::AddGrass(const std::vector<glm::vec3>& v)
{
	grassPositions.insert(grassPositions.end(), v.begin(), v.end());
	grassVBO->SetData(grassPositions.data(), maxGrassBlades * sizeof(glm::vec3));
}

// TODO: Grass "curves"
// TODO: Add "grass submissions". Will basically server as grass clusters where we can vary things light grass thickness/height, wind strength, textures, etc
// TODO: Add rotation around Y axis to grass vertex data