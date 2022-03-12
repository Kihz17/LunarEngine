#include "EnvironmentMapPass.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "ShaderLibrary.h"
#include "RenderBuffer.h"
#include "Renderer.h"

const std::string EnvironmentMapPass::CUBE_MAP_DRAW_SHADER_KEY = "drawEnvShader";

EnvironmentMapPass::EnvironmentMapPass(const WindowSpecs* windowSpecs)
	: environmentBuffer(new FrameBuffer()),
	shader(ShaderLibrary::Load(CUBE_MAP_DRAW_SHADER_KEY, "assets/shaders/environmentBuffer.glsl")),
	envMapTexture(TextureManager::CreateTexture2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, windowSpecs->width, windowSpecs->height, TextureFilterType::Linear, TextureWrapType::None)),
	windowSpecs(windowSpecs)
{
	// Setup color attachements for FBO
	environmentBuffer->Bind();
	environmentBuffer->AddColorAttachment2D("environment", envMapTexture, 0);
	environmentBuffer->Unbind();

	// Setup shader uniforms
	shader->Bind();
	shader->InitializeUniform("uProjection");
	shader->InitializeUniform("uView");
	shader->InitializeUniform("uEnvMap");
	shader->SetInt("uEnvMap", 0);
	shader->Unbind();
}

EnvironmentMapPass::~EnvironmentMapPass()
{
	delete environmentBuffer;
}

void EnvironmentMapPass::DoPass(CubeMap* cubeMap, const glm::mat4& projection, const glm::mat4& view, PrimitiveShape& cube)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE); // Make sure none of our cubes faces get culled

	environmentBuffer->Bind();
	shader->Bind();
	shader->SetMat4("uProjection", projection);
	shader->SetMat4("uView", view);
	cubeMap->BindToSlot(0);
	shader->SetInt("uEnvMap", 0);
	cube.Draw();
	environmentBuffer->Unbind();

	glEnable(GL_CULL_FACE); // Re-enable face culling
}