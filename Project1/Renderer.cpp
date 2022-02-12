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
Frustum Renderer::viewFrustum;

static std::vector<RenderSubmission> submissions;
static std::vector<RenderSubmission> forwardSubmissions;
static std::vector<LineRenderSubmission> lineSubmissions;

float Renderer::farPlane = 1000.0f;
float Renderer::nearPlane = 0.1f;

GeometryPass* Renderer::geometryPass = nullptr;
EnvironmentMapPass* Renderer::envMapPass = nullptr;
LightingPass* Renderer::lightingPass = nullptr;
ForwardRenderPass* Renderer::forwardPass = nullptr;
CascadedShadowMapping* Renderer::shadowMappingPass = nullptr;
LinePass* Renderer::linePass = nullptr;

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

	geometryPass = new GeometryPass(windowDetails);

	CascadedShadowMappingInfo csmInfo(view, camera.fov, nearPlane, farPlane);
	csmInfo.windowSpecs = windowDetails;
	csmInfo.zMult = 10.0f;
	shadowMappingPass = new CascadedShadowMapping(csmInfo);
	envMapPass = new EnvironmentMapPass(windowDetails);
	lightingPass = new LightingPass(geometryPass->GetGBuffer(), envMapPass->GetEnvironmentBuffer(), windowDetails, shadowMappingPass->GetShadowMap(), shadowMappingPass->GetCascadeLevels());
	forwardPass = new ForwardRenderPass(geometryPass->GetGBuffer());
	linePass = new LinePass();

	// TEST SECITON FOR GRASS
	//{
	//	grassVAO = new VertexArrayObject();
	//	//grassVBO = new VertexBuffer(nullptr, MAX_GRASS_BLADES * sizeof(float) * 3);

	//	float positions[100 * 3];
	//	int currentIndex = 0;
	//	for (int i = 0; i < 100; i++)
	//	{
	//		positions[currentIndex] = i;
	//		positions[currentIndex + 1] = 10.0f;
	//		positions[currentIndex + 2] = 0.0f;
	//		currentIndex += 3;
	//	}
	//	grassVBO = new VertexBuffer(positions, 300 * sizeof(float));

	//	BufferLayout bufferLayout = {
	//		{ ShaderDataType::Float3, "vWorldPosition" }
	//	};
	//	grassVBO->SetLayout(bufferLayout);
	//	grassVAO->AddVertexBuffer(grassVBO);

	//	// Grass shader
	//	Shader* grassShader = ShaderLibrary::Load("grassShader", "assets/shaders/grass.glsl");
	//	grassShader->Bind();
	//	grassShader->InitializeUniform("uWidthHeight");
	//	grassShader->InitializeUniform("uLODLevel");
	//	grassShader->InitializeUniform("uAlbedoTexture");
	//	grassShader->InitializeUniform("uHasNormalTexture");
	//	grassShader->InitializeUniform("uNormalTexture");
	//	grassShader->InitializeUniform("uRoughnessTexture");
	//	grassShader->InitializeUniform("uMetalnessTexture");
	//	grassShader->InitializeUniform("uAmbientOcculsionTexture");
	//	grassShader->InitializeUniform("uMaterialOverrides");
	//	grassShader->InitializeUniform("uDiscardTexture");
	//	grassShader->Unbind();
	//}
}

void Renderer::CleanUp()
{
	delete geometryPass;
	delete envMapPass;
	delete lightingPass;
	delete forwardPass;
	delete shadowMappingPass;
	delete linePass;
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

	float aspect = (float)windowDetails->width / (float)windowDetails->height;
	projection = glm::perspective(camera.fov, aspect, nearPlane, farPlane);
	view = camera.GetViewMatrix();
	cameraPos = camera.position;
	viewFrustum = FrustumUtils::CreateFrustumFromCamera(camera, aspect, farPlane, nearPlane);
}

void Renderer::EndFrame()
{
	submissions.clear();
	forwardSubmissions.clear();

	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame()
{
	// Cull if not in frustum
	std::vector<RenderSubmission> culledSubmissions;
	for (RenderSubmission& submission : submissions)
	{
		if (!submission.renderComponent->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) continue; // Not in our view frustum, no need to render
		culledSubmissions.push_back(submission);
	}

	std::vector<RenderSubmission> culledForwardSubmissions;
	for (RenderSubmission& submission : forwardSubmissions)
	{
		if (!submission.renderComponent->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) continue; // Not in our view frustum, no need to render
		culledForwardSubmissions.push_back(submission);
	}

	// Some passes need both
	std::vector<RenderSubmission> mixedCulledSubmissions;
	mixedCulledSubmissions.insert(mixedCulledSubmissions.end(), culledSubmissions.begin(), culledSubmissions.end());
	mixedCulledSubmissions.insert(mixedCulledSubmissions.end(), culledForwardSubmissions.begin(), culledForwardSubmissions.end());

	geometryPass->DoPass(culledSubmissions, projection, view, cameraPos);
	shadowMappingPass->DoPass(mixedCulledSubmissions, projection, view);
	envMapPass->DoPass(projection, view);
	lightingPass->DoPass(culledSubmissions, projection, view, cameraPos);
	forwardPass->DoPass(culledForwardSubmissions, projection, view, windowDetails);
	linePass->DoPass(lineSubmissions, projection, view, windowDetails);
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

void Renderer::SubmitLines(const LineRenderSubmission& submission)
{
	lineSubmissions.push_back(submission);
}

void Renderer::SetEnvironmentMapEquirectangular(const std::string& path)
{
	envMapPass->SetEnvironmentMapEquirectangular(path);
}

void Renderer::SetShadowMappingDirectionalLight(Light* light)
{
	shadowMappingPass->SetDirectionalLight(light);
}