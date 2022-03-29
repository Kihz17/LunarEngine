#include "GrassPass.h"
#include "ShaderLibrary.h"
#include "TextureManager.h"
#include "Utils.h"

#include <vendor/imgui/imgui.h>

const std::string GrassPass::GRASS_SHADER_KEY = "grassShader";

GrassPass::GrassPass()
	: shader(ShaderLibrary::Load(GRASS_SHADER_KEY, "assets/shaders/grass.glsl"))
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
}

GrassPass::~GrassPass()
{

}

void GrassPass::DoPass(IFrameBuffer* geometryBuffer, std::vector<GrassCluster>& grassClusters, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view)
{
	glDisable(GL_CULL_FACE); // Don't face cull, we want to render both sides of grass blades
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	geometryBuffer->Bind();

	shader->Bind();

	shader->SetFloat3("uCameraPosition", cameraPos);
	shader->SetMat4("uProjection", proj);
	shader->SetMat4("uView", view);
	shader->SetFloat("uTime", glfwGetTime());

	for (GrassCluster& grassCluster : grassClusters)
	{
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
	}

	geometryBuffer->Unbind();

	glDisable(GL_MULTISAMPLE);
}