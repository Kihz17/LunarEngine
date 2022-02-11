#include "Renderer.h"
#include "ShaderLibrary.h"
#include "EntityManager.h"

#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "CubeMap.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <sstream>

const WindowSpecs* Renderer::windowDetails = nullptr;

glm::mat4 Renderer::projection(1.0f);
glm::mat4 Renderer::view(1.0f);
glm::vec3 Renderer::cameraPos(0.0f);

static std::vector<RenderSubmission> submissions;
static std::vector<RenderSubmission> forwardSubmissions;

float Renderer::farPlane = 500.0f;
float Renderer::nearPlane = 0.1f;

GeometryPass* Renderer::geometryPass = nullptr;
EnvironmentMapPass* Renderer::envMapPass = nullptr;
LightingPass* Renderer::lightingPass = nullptr;
ForwardRenderPass* Renderer::forwardPass = nullptr;
CascadedShadowMapping* Renderer::shadowMappingPass = nullptr;

const std::string Renderer::LIGHTING_SHADER_KEY = "lShader";
const std::string Renderer::FORWARD_SHADER_KEY = "fShader";

static const uint32_t MAX_GRASS_BLADES = 2048;
static VertexArrayObject* grassVAO = nullptr;
static VertexBuffer* grassVBO = nullptr;
static uint32_t grassCount = 0;
static float* grassRoots = new float[MAX_GRASS_BLADES]; // Allows for 2048 grass blades (Since this array is so big, i'm allocating on heap instead of stack)

void Renderer::Initialize(const Camera& camera, WindowSpecs* window)
{
	windowDetails = window;

	geometryPass = new GeometryPass(windowDetails, cameraPos);
	envMapPass = new EnvironmentMapPass(windowDetails);

	CascadedShadowMappingInfo csmInfo(view, camera.fov, nearPlane, farPlane);
	csmInfo.windowSpecs = windowDetails;
	csmInfo.zMult = 10.0f;
	shadowMappingPass = new CascadedShadowMapping(csmInfo);

	lightingPass = new LightingPass(geometryPass->GetGBuffer(), envMapPass->GetEnvironmentBuffer(), windowDetails, cameraPos, shadowMappingPass->GetShadowMap(), shadowMappingPass->GetCascadeLevels());
	forwardPass = new ForwardRenderPass(geometryPass->GetGBuffer(), windowDetails);

	// TEST SECITON FOR GRASS
	{
		grassVAO = new VertexArrayObject();
		//grassVBO = new VertexBuffer(nullptr, MAX_GRASS_BLADES * sizeof(float) * 3);

		float positions[100 * 3];
		int currentIndex = 0;
		for (int i = 0; i < 100; i++)
		{
			positions[currentIndex] = i;
			positions[currentIndex + 1] = 10.0f;
			positions[currentIndex + 2] = 0.0f;
			currentIndex += 3;
		}
		grassVBO = new VertexBuffer(positions, 300 * sizeof(float));

		BufferLayout bufferLayout = {
			{ ShaderDataType::Float3, "vWorldPosition" }
		};
		grassVBO->SetLayout(bufferLayout);
		grassVAO->AddVertexBuffer(grassVBO);

		// Grass shader
		Shader* grassShader = ShaderLibrary::Load("grassShader", "assets/shaders/grass.glsl");
		grassShader->Bind();
		grassShader->InitializeUniform("uWidthHeight");
		grassShader->InitializeUniform("uLODLevel");
		grassShader->InitializeUniform("uAlbedoTexture");
		grassShader->InitializeUniform("uHasNormalTexture");
		grassShader->InitializeUniform("uNormalTexture");
		grassShader->InitializeUniform("uRoughnessTexture");
		grassShader->InitializeUniform("uMetalnessTexture");
		grassShader->InitializeUniform("uAmbientOcculsionTexture");
		grassShader->InitializeUniform("uMaterialOverrides");
		grassShader->InitializeUniform("uDiscardTexture");
		grassShader->Unbind();
	}
}

void Renderer::CleanUp()
{

}

void Renderer::SetViewType(uint32_t type)
{
	if (type <= 0 || type >= 9) return; // Out of range

	const Shader* shader = ShaderLibrary::Get(LIGHTING_SHADER_KEY);
	shader->Bind();
	shader->SetInt("uViewType", type);
	shader->Unbind();
}

void Renderer::BeginFrame(const Camera& camera)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection = glm::perspective(camera.fov, (float)windowDetails->width / (float)windowDetails->height, nearPlane, farPlane);
	view = camera.GetViewMatrix();
	cameraPos = camera.position;
}

void Renderer::EndFrame()
{
	submissions.clear();
	forwardSubmissions.clear();

	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame()
{
	geometryPass->DoPass(submissions, projection, view);
	shadowMappingPass->DoPass(submissions, projection, view);
	envMapPass->DoPass(submissions, projection, view);
	lightingPass->DoPass(submissions, projection, view);
	forwardPass->DoPass(forwardSubmissions, projection, view);
}

void Renderer::Submit(const RenderSubmission& submission)
{
	RenderComponent* renderComponent = submission.renderComponent;
	if (renderComponent->alphaTransparency < 1.0f || renderComponent->isIgnoreLighting) // Needs alpha blending, use this in the forward pass
	{
		forwardSubmissions.push_back(submission);
	}
	else
	{
		submissions.push_back(submission);
	}
}

void Renderer::SetEnvironmentMapEquirectangular(const std::string& path)
{
	envMapPass->SetEnvironmentMapEquirectangular(path);
}

void Renderer::SetShadowMappingDirectionalLight(Light* light)
{
	shadowMappingPass->SetDirectionalLight(light);
}