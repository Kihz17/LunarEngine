#include "Renderer.h"
#include "ShaderLibrary.h"
#include "EntityManager.h"
#include "Utils.h"
#include "Profiler.h"

#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "CubeMap.h"
#include "EquirectangularToCubeMapConverter.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <sstream>

const WindowSpecs* Renderer::windowDetails = nullptr;

glm::mat4 Renderer::projection(1.0f);
glm::mat4 Renderer::view(1.0f);
glm::vec3 Renderer::cameraPos(0.0f);
glm::vec3 cameraDir(0.0f);
Frustum Renderer::viewFrustum;
float Renderer::shadowCullRadius = 1000.0f;

std::vector<RenderSubmission> Renderer::culledShadowSubmissions;
std::vector<RenderSubmission> Renderer::culledSubmissions;
std::vector<RenderSubmission> Renderer::culledAnimatedSubmissions;
std::vector<RenderSubmission> Renderer::culledAnimatedShadowSubmissions;
std::vector<RenderSubmission> Renderer::culledForwardSubmissions;
std::vector<LineRenderSubmission> Renderer::lineSubmissions;

float Renderer::farPlane = 1000.0f;
float Renderer::nearPlane = 0.1f;

GeometryPass* Renderer::geometryPass = nullptr;
EnvironmentMapPass* Renderer::envMapPass = nullptr;
LightingPass* Renderer::lightingPass = nullptr;
ForwardRenderPass* Renderer::forwardPass = nullptr;
CascadedShadowMapping* Renderer::shadowMappingPass = nullptr;
LinePass* Renderer::linePass = nullptr;
CloudPass* Renderer::cloudPass = nullptr;
ProceduralGrassPass* Renderer::grassPass = nullptr;
TerrainPass* Renderer::terrainPass = nullptr;

TerrainGenerationInfo Renderer::terrainInfo;
GrassCluster Renderer::grassCluster;

CubeMap* Renderer::envMap = nullptr;
DynamicCubeMapRenderer* Renderer::dynamicCubeMapGenerator = nullptr;

const std::string Renderer::LIGHTING_SHADER_KEY = "lShader";
const std::string Renderer::FORWARD_SHADER_KEY = "fShader";

PrimitiveShape* Renderer::quad = nullptr;
PrimitiveShape* Renderer::cube = nullptr;

Light* Renderer::mainLight = nullptr;

static const uint32_t MAX_GRASS_BLADES = 2048;
static VertexArrayObject* grassVAO = nullptr;
static VertexBuffer* grassVBO = nullptr;
static uint32_t grassCount = 0;
static float* grassRoots = new float[MAX_GRASS_BLADES]; // Allows for 2048 grass blades (Since this array is so big, i'm allocating on heap instead of stack)

void Renderer::Initialize(const Camera& camera, WindowSpecs* window)
{
	windowDetails = window;

	Renderer::quad = new PrimitiveShape(ShapeType::Quad);
	Renderer::cube = new PrimitiveShape(ShapeType::Cube);

	geometryPass = new GeometryPass(windowDetails);

	CascadedShadowMappingInfo csmInfo(view, camera.fov, nearPlane, farPlane);
	csmInfo.windowSpecs = windowDetails;
	csmInfo.zMult = 10.0f;
	shadowMappingPass = new CascadedShadowMapping(csmInfo);
	envMapPass = new EnvironmentMapPass(windowDetails);
	lightingPass = new LightingPass(windowDetails, shadowMappingPass->GetShadowMap(), shadowMappingPass->GetCascadeLevels());
	forwardPass = new ForwardRenderPass(geometryPass->GetGBuffer());
	linePass = new LinePass();
	terrainPass = new TerrainPass();
	cloudPass = new CloudPass(100000.0f, 1000.0f, 3000.0f, 0.0004f, windowDetails);
	grassPass = new ProceduralGrassPass();

	dynamicCubeMapGenerator = new DynamicCubeMapRenderer(windowDetails);

	EquirectangularToCubeMapConverter::Initialize();

	// Create grass blades
	unsigned int numGrassBlades = 3000000;
	grassCluster.grassData.resize(numGrassBlades);
	for (int i = 0; i < numGrassBlades; i++)
	{
		grassCluster.grassData[i] = glm::vec4(Utils::RandFloat(-300.0f, 300.0f), 0.0f, Utils::RandFloat(-300.0f, 300.0f), glm::radians(Utils::RandFloat(0.0f, 360.0f)));
	}


	BufferLayout bufferLayout = {
		{ ShaderDataType::Float4, "vWorldPosition" }
	};

	grassCluster.VAO = new VertexArrayObject();
	grassCluster.VBO = new VertexBuffer(numGrassBlades * sizeof(glm::vec4));
	grassCluster.VBO->SetData(grassCluster.grassData.data(), numGrassBlades * sizeof(glm::vec4));
	grassCluster.VBO->SetLayout(bufferLayout);
	grassCluster.VAO->AddVertexBuffer(grassCluster.VBO);
}

void Renderer::CleanUp()
{
	delete geometryPass;
	delete envMapPass;
	delete lightingPass;
	delete forwardPass;
	delete shadowMappingPass;
	delete linePass;
	delete terrainPass;
	delete dynamicCubeMapGenerator;
	delete cloudPass;
	delete grassPass;

	delete Renderer::quad;
	delete Renderer::cube;

	EquirectangularToCubeMapConverter::CleanUp();
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
	Profiler::BeginProfile("BeginFrame");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = (float)windowDetails->width / (float)windowDetails->height;
	projection = glm::perspective(camera.fov, aspect, nearPlane, farPlane);
	view = camera.GetViewMatrix();
	cameraPos = camera.position;
	cameraDir = camera.front;
	viewFrustum = FrustumUtils::CreateFrustumFromCamera(camera, aspect, farPlane, nearPlane);

	Profiler::EndProfile("BeginFrame");
}

void Renderer::EndFrame()
{
	Profiler::BeginProfile("EndFrame");

	culledSubmissions.clear();
	culledShadowSubmissions.clear();

	culledAnimatedSubmissions.clear();
	culledAnimatedShadowSubmissions.clear();

	culledForwardSubmissions.clear();
	lineSubmissions.clear();

	glfwSwapBuffers(windowDetails->window);
	Profiler::EndProfile("EndFrame");
}

void Renderer::DrawFrame()
{
	Profiler::BeginProfile("DrawFrame");

	Profiler::BeginProfile("GeometryPass");
	geometryPass->DoPass(culledSubmissions, culledAnimatedSubmissions, projection, view, cameraPos);
	Profiler::EndProfile("GeometryPass");

	Profiler::BeginProfile("TerrainPass");
	//terrainPass->DoPass(geometryPass->GetGBuffer(), terrainInfo, projection, view, cameraPos);
	Profiler::EndProfile("TerrainPass");

	Profiler::BeginProfile("GrassPass");
	//grassPass->DoPass(geometryPass->GetGBuffer(), grassCluster, terrainInfo, cameraPos, projection, view);
	Profiler::EndProfile("GrassPass");

	if (envMap) // Only do env map pass if we have one
	{
		glm::vec3 lightDir = mainLight ? mainLight->direction : glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 lightColor = mainLight ? mainLight->color : glm::vec3(0.0f, 0.0f, 0.0f);

		Profiler::BeginProfile("EnvMapPass");
		envMapPass->DoPass(envMap, projection, view, mainLight, cameraPos, glm::normalize(lightDir), lightColor, cube);
		Profiler::EndProfile("EnvMapPass");
	}

	if (mainLight) // Can't do clouds & shadows without a light source
	{
		glm::vec3 lightDir = glm::normalize(mainLight->direction);

		Profiler::BeginProfile("CloudPass");
		cloudPass->DoPass(envMapPass->GetEnvironmentTexture(), geometryPass->GetPositionBuffer(), projection, view, cameraPos, lightDir, mainLight->color, cameraDir, windowDetails, quad);
		Profiler::EndProfile("CloudPass");

		Profiler::BeginProfile("ShadowPass");
		shadowMappingPass->DoPass(culledShadowSubmissions, culledAnimatedShadowSubmissions, lightDir, projection, view, *quad);
		Profiler::EndProfile("ShadowPass");
	}
	
	Profiler::BeginProfile("LightingPass");
	lightingPass->DoPass(geometryPass->GetPositionBuffer(), geometryPass->GetAlbedoBuffer(), 
		geometryPass->GetNormalBuffer(), geometryPass->GetEffectsBuffer(),
		cloudPass->GetSkyTexture(), shadowMappingPass->GetSoftnessTexture(), projection, view, cameraPos, *quad);
	Profiler::EndProfile("LightingPass");

	forwardPass->DoPass(culledForwardSubmissions, projection, view, windowDetails);
	linePass->DoPass(lineSubmissions, projection, view, windowDetails);

	Profiler::EndProfile("DrawFrame");
}

CubeMap* Renderer::GenerateDynamicCubeMap(const glm::vec3& center, ReflectRefractMapPriorityType meshPriority, RenderComponent* ignore, int viewportWidth, int viewportHeight)
{
	return nullptr;
	//constexpr float fov = glm::radians(90.0f);
	//constexpr float aspect = 1.0f; // Since we are making a cube map, our aspect ratio is 1:1

	//Frustum frontFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::FrontVec(), Utils::UpVec(), Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	//Frustum backFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::FrontVec(), Utils::UpVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	//Frustum leftFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::RightVec(), Utils::UpVec(), Utils::FrontVec(), fov, aspect, farPlane, nearPlane);
	//Frustum rightFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::RightVec(), Utils::UpVec(), -Utils::FrontVec(), fov, aspect, farPlane, nearPlane);
	//Frustum upFrustum = FrustumUtils::CreateFrustumFromCamera(center, Utils::UpVec(), -Utils::FrontVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);
	//Frustum downFrustum = FrustumUtils::CreateFrustumFromCamera(center, -Utils::UpVec(), Utils::FrontVec(), -Utils::RightVec(), fov, aspect, farPlane, nearPlane);

	//std::unordered_map<int, std::vector<RenderSubmission*>> dynamicSubmissions;
	//for (int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++)
	//{
	//	dynamicSubmissions.insert({ i, std::vector<RenderSubmission*>() });
	//}

	//for (RenderSubmission& submission : submissions)
	//{
	//	if (submission.renderComponent->reflectRefractMapPriority > meshPriority || submission.renderComponent == ignore) continue; // This mesh shouldn't be considered in this dynamic cube map

	//	// Cull objects that aren't within their respective frustums
	//	const AABB* aabb = submission.renderComponent->mesh->GetBoundingBox();
	//	if (aabb->IsOnFrustum(frontFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(backFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Z].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(leftFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_X].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(rightFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_X].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(upFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Y].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(downFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y].push_back(&submission);
	//	}
	//}

	//for (RenderSubmission& submission : animatedSubmissions)
	//{
	//	if (submission.renderComponent->reflectRefractMapPriority > meshPriority || submission.renderComponent == ignore) continue; // This mesh shouldn't be considered in this dynamic cube map

	//	// Cull objects that aren't within their respective frustums
	//	const AABB* aabb = submission.renderComponent->mesh->GetBoundingBox();
	//	if (aabb->IsOnFrustum(frontFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(backFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Z].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(leftFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_X].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(rightFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_X].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(upFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_POSITIVE_Y].push_back(&submission);
	//	}
	//	if (aabb->IsOnFrustum(downFrustum, submission.transform))
	//	{
	//		dynamicSubmissions[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y].push_back(&submission);
	//	}
	//}

	//CubeMap* cm = dynamicCubeMapGenerator->GenerateDynamicCubeMap(center, dynamicSubmissions, fov, farPlane, nearPlane, windowDetails);
	//glViewport(0, 0, viewportWidth, viewportHeight);
	//return cm;
}