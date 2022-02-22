#include "Renderer.h"
#include "ShaderLibrary.h"
#include "EntityManager.h"
#include "Utils.h"
#include "Model.h"

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
float Renderer::shadowCullRadius = 1000.0f;

static std::vector<RenderSubmission> submissions;
static std::vector<RenderSubmission> animatedSubmissions;
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

DynamicCubeMapRenderer* Renderer::dynamicCubeMapGenerator = nullptr;

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

	dynamicCubeMapGenerator = new DynamicCubeMapRenderer(windowDetails);

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
	delete dynamicCubeMapGenerator;
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
	std::vector<RenderSubmission*> culledShadowSubmissions;
	std::vector<RenderSubmission*> culledSubmissions;
	std::vector<RenderSubmission*> culledAnimatedSubmissions;
	std::vector<RenderSubmission*> culledForwardSubmissions;

	// Cull deferred submissions
	for (RenderSubmission& submission : submissions)
	{
		if (submission.submesh->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledShadowSubmissions.push_back(&submission);
		}
	}

	// Cull animated submissions
	for (RenderSubmission& submission : animatedSubmissions)
	{
		if (submission.submesh->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledAnimatedSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledShadowSubmissions.push_back(&submission);
		}
	}

	// Cull forward submissions
	for (RenderSubmission& submission : forwardSubmissions)
	{
		if (!submission.submesh->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledForwardSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledShadowSubmissions.push_back(&submission);
		}
	}

	geometryPass->DoPass(culledSubmissions, culledAnimatedSubmissions, projection, view, cameraPos);
	shadowMappingPass->DoPass(culledShadowSubmissions, projection, view);
	envMapPass->DoPass(projection, view);
	lightingPass->DoPass(projection, view, cameraPos);
	forwardPass->DoPass(culledForwardSubmissions, projection, view, windowDetails);
	linePass->DoPass(lineSubmissions, projection, view, windowDetails);
}

void Renderer::Submit(const RenderSubmission& submission)
{
	Submesh* submesh = submission.submesh;
	if (submesh->alphaTransparency < 1.0f || submesh->isIgnoreLighting) // Needs alpha blending, use this in the forward pass
	{
		forwardSubmissions.push_back(submission);
	}
	else if (submission.boneMatrices) // This submission has an animation
	{
		animatedSubmissions.push_back(submission);
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

CubeMap* Renderer::GenerateDynamicCubeMap(const glm::vec3& center, ReflectRefractMapPriorityType meshPriority, Submesh* ignore, int viewportWidth, int viewportHeight)
{
	constexpr float fov = glm::radians(90.0f);
	constexpr float aspect = 1.0f; // Since we are making a cube map, our aspect ratio is 1:1

	Frustum frontFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::FrontVec(), Utils::UpVec(), Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	Frustum backFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::FrontVec(), Utils::UpVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	Frustum leftFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::RightVec(), Utils::UpVec(), Utils::FrontVec(), fov, aspect, farPlane, nearPlane);
	Frustum rightFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::RightVec(), Utils::UpVec(), -Utils::FrontVec(), fov, aspect, farPlane, nearPlane);
	Frustum upFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::UpVec(), -Utils::FrontVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	Frustum downFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::UpVec(), Utils::FrontVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);

	std::unordered_map<int, std::vector<RenderSubmission*>> dynamicSubmissions;
	for (int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++)
	{
		dynamicSubmissions.insert({ i, std::vector<RenderSubmission*>() });
	}

	for (RenderSubmission& submission : submissions)
	{
		if (submission.submesh->reflectRefractMapPriority > meshPriority || submission.submesh == ignore) continue; // This mesh shouldn't be considered in this dynamic cube map
		
		// Cull objects that aren't within their respective frustums
		const AABB* aabb = submission.submesh->mesh->GetBoundingBox();
		if (aabb->IsOnFrustum(frontFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z].push_back(&submission);
		}
		if (aabb->IsOnFrustum(backFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Z].push_back(&submission);
		}
		if (aabb->IsOnFrustum(leftFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_X].push_back(&submission);
		}
		if (aabb->IsOnFrustum(rightFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_X].push_back(&submission);
		}
		if (aabb->IsOnFrustum(upFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Y].push_back(&submission);
		}
		if (aabb->IsOnFrustum(downFrustum, submission.transform))
		{
			dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y].push_back(&submission);
		}
	}

	CubeMap* cm = dynamicCubeMapGenerator->GenerateDynamicCubeMap(center, dynamicSubmissions, fov, farPlane, nearPlane, windowDetails);
	glViewport(0, 0, viewportWidth, viewportHeight);
	return cm;
}