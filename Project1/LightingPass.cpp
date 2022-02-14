#include "LightingPass.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "RenderBuffer.h"
#include "ShaderLibrary.h"
#include "Light.h"
#include "Renderer.h"
#include "CascadedShadowMapping.h"

#include <sstream>

LightingPass::LightingPass(IFrameBuffer* gBuffer, IFrameBuffer* eBuffer, const WindowSpecs* windowSpecs, ITexture* shadowMaps, std::vector<float>& cascadeLevels)
	: geometryBuffer(gBuffer),
	environmentBuffer(eBuffer),
	shader(ShaderLibrary::Load(Renderer::LIGHTING_SHADER_KEY, "assets/shaders/brdfLighting.glsl")), 
	shadowMaps(shadowMaps),
	cascadeLevels(cascadeLevels),
	quad(ShapeType::Quad)
{
	// Setup shader uniforms
	shader->Bind();
	shader->InitializeUniform("uInverseView");
	shader->InitializeUniform("uInverseProjection");
	shader->InitializeUniform("uView");
	shader->InitializeUniform("gPosition");
	shader->InitializeUniform("gAlbedo");
	shader->InitializeUniform("gNormal");
	shader->InitializeUniform("gEffects");
	shader->InitializeUniform("uLightAmount");

	for (int i = 0; i < Light::MAX_LIGHTS; i++)
	{
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].position";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].direction";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].color";
			shader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].param1";
			shader->InitializeUniform(ss.str());
		}
	}

	shader->InitializeUniform("uEnvMap");
	shader->InitializeUniform("uViewType");
	shader->InitializeUniform("uReflectivity");
	shader->InitializeUniform("uCameraPosition");
	shader->InitializeUniform("uShadowMap");
	for (int i = 0; i < CascadedShadowMapping::MAX_CASCADE_LEVELS; i++)
	{
		shader->InitializeUniform(std::string("uCascadePlaneDistances[" + std::to_string(i) + "]"));
	}
	shader->InitializeUniform("uCascadeCount");

	shader->SetInt("uViewType", 1); // Regular color view by default

	// Set samplers for lighting
	shader->SetInt("gPosition", 0);
	shader->SetInt("gAlbedo", 1);
	shader->SetInt("gNormal", 2);
	shader->SetInt("gEffects", 3);
	shader->SetInt("uEnvMap", 4);
	shader->SetInt("uShadowMap", 5);

	shader->Unbind();
}

LightingPass::~LightingPass()
{
	
}

void LightingPass::DoPass(std::vector<RenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glDisable(GL_DEPTH_TEST); // Disable depth buffer so that the quad doesnt get discarded

	shader->Bind();

	// Bind G-Buffer textures
	geometryBuffer->GetColorAttachment("position")->BindToSlot(0);
	geometryBuffer->GetColorAttachment("albedo")->BindToSlot(1);
	geometryBuffer->GetColorAttachment("normal")->BindToSlot(2);
	geometryBuffer->GetColorAttachment("effects")->BindToSlot(3);

	// Bind environment map stuff
	environmentBuffer->GetColorAttachment("environment")->BindToSlot(4);

	// Bind shadow map
	shadowMaps->BindToSlot(5);

	// Shadow mapping details
	shader->SetInt("uCascadeCount", cascadeLevels.size());
	for (size_t i = 0; i < cascadeLevels.size(); i++)
	{
		shader->SetFloat("uCascadePlaneDistances[" + std::to_string(i) + "]", cascadeLevels[i]);
	}

	shader->SetMat4("uInverseView", glm::transpose(view));
	shader->SetMat4("uInverseProjection", glm::inverse(projection));
	shader->SetMat4("uView", view);
	shader->SetFloat3("uCameraPosition", cameraPosition);

	// TODO: Move these into gBuffer so it can be passed in with the geometry
	shader->SetFloat3("uReflectivity", glm::vec3(0.04f));

	quad.Draw();

	glEnable(GL_DEPTH_TEST);
}
