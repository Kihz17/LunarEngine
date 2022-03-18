#include "Renderer.h"
#include "ShaderLibrary.h"
#include "EntityManager.h"
#include "Utils.h"

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
CloudPass* Renderer::cloudPass = nullptr;

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
	cloudPass = new CloudPass(windowDetails);

	dynamicCubeMapGenerator = new DynamicCubeMapRenderer(windowDetails);

	EquirectangularToCubeMapConverter::Initialize();

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
	delete cloudPass;

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = (float)windowDetails->width / (float)windowDetails->height;
	projection = glm::perspective(camera.fov, aspect, nearPlane, farPlane);
	view = camera.GetViewMatrix();
	cameraPos = camera.position;
	cameraDir = camera.front;
	viewFrustum = FrustumUtils::CreateFrustumFromCamera(camera, aspect, farPlane, nearPlane);
}

void Renderer::EndFrame()
{
	submissions.clear();
	animatedSubmissions.clear();
	forwardSubmissions.clear();
	lineSubmissions.clear();

	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame()
{
	std::vector<RenderSubmission*> culledShadowSubmissions;
	std::vector<RenderSubmission*> culledSubmissions;
	std::vector<RenderSubmission*> culledAnimatedSubmissions;
	std::vector<RenderSubmission*> culledAnimatedShadowSubmissions;
	std::vector<RenderSubmission*> culledForwardSubmissions;

	std::vector<std::pair<RenderComponent*, CubeMap*>> dynamicCubeMaps;

	// Cull deferred submissions
	for (RenderSubmission& submission : submissions)
	{
		if (submission.renderComponent->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledShadowSubmissions.push_back(&submission);
		}

		// Generate dynamic cubemaps if we should
		if (submission.renderComponent->reflectRefractData.type != ReflectRefractType::None)
		{
			ReflectRefractMapType mapType = submission.renderComponent->reflectRefractData.mapType;

			if (mapType != ReflectRefractMapType::Custom && mapType != ReflectRefractMapType::Environment) // Custom maps aren't created on the fly
			{
				ReflectRefractMapPriorityType mesPriority = mapType == ReflectRefractMapType::DynamicFull ? ReflectRefractMapPriorityType::Low
					: mapType == ReflectRefractMapType::DynamicMedium ? ReflectRefractMapPriorityType::Medium : ReflectRefractMapPriorityType::High;

				CubeMap* cubeMap = GenerateDynamicCubeMap(submission.renderComponent->mesh->GetBoundingBox()->GetCenter(), mesPriority, submission.renderComponent);
				dynamicCubeMaps.push_back(std::make_pair(submission.renderComponent, cubeMap)); // Store so we can delete after this frame is over
				submission.renderComponent->reflectRefractData.customMap = cubeMap;
			}
		}
	}

	// Cull animated submissions
	for (RenderSubmission& submission : animatedSubmissions)
	{
		if (submission.renderComponent->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledAnimatedSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledAnimatedShadowSubmissions.push_back(&submission);
		}
	}

	// Cull forward submissions
	for (RenderSubmission& submission : forwardSubmissions)
	{
		if (!submission.renderComponent->mesh->GetBoundingBox()->IsOnFrustum(viewFrustum, submission.transform)) // In our view frustum, we should render
		{
			culledForwardSubmissions.push_back(&submission);
		}

		if (glm::length(cameraPos - glm::vec3(submission.transform[3])) <= shadowCullRadius) // We are within shadow radius, show shadows
		{
			culledShadowSubmissions.push_back(&submission);
		}
	}

	geometryPass->DoPass(culledSubmissions, culledAnimatedSubmissions, projection, view, cameraPos);


	if (envMap) // Only do env map pass if we have one
	{
		glm::vec3 lightDir = mainLight ? mainLight->direction : glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 lightColor = mainLight ? mainLight->color : glm::vec3(0.0f, 0.0f, 0.0f);
		envMapPass->DoPass(envMap, projection, view, mainLight, cameraPos, glm::normalize(lightDir), lightColor, cube);
	}

	if (mainLight) // Can't do clouds & shadows without a light source
	{
		glm::vec3 lightDir = glm::normalize(mainLight->direction);
		cloudPass->DoPass(envMapPass->GetEnvironmentTexture(), projection, view, cameraPos, lightDir, mainLight->color, cameraDir, windowDetails, quad);
		shadowMappingPass->DoPass(culledShadowSubmissions, culledAnimatedShadowSubmissions, lightDir, projection, view, *quad);
	}
	
	lightingPass->DoPass(geometryPass->GetPositionBuffer(), geometryPass->GetAlbedoBuffer(), 
		geometryPass->GetNormalBuffer(), geometryPass->GetEffectsBuffer(),
		cloudPass->GetSkyTexture(), shadowMappingPass->GetSoftnessTexture(), projection, view, cameraPos, *quad);

	forwardPass->DoPass(culledForwardSubmissions, projection, view, windowDetails);
	linePass->DoPass(lineSubmissions, projection, view, windowDetails);

	// Free up dynamic cube map space
	for (std::pair<RenderComponent*, CubeMap*>& pair : dynamicCubeMaps)
	{
		TextureManager::DeleteTexture(pair.second); // Free the cube map from memory
		pair.first->reflectRefractData.customMap = nullptr; // Make sure we unset the custom cube map incase we aren't using a dynamic cube map next frame
	}
}

void Renderer::Submit(const RenderSubmission& submission)
{
	RenderComponent* renderComponent = submission.renderComponent;
	if (renderComponent->alphaTransparency < 1.0f || renderComponent->isIgnoreLighting) // Needs alpha blending, use this in the forward pass
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

CubeMap* Renderer::GenerateDynamicCubeMap(const glm::vec3& center, ReflectRefractMapPriorityType meshPriority, RenderComponent* ignore, int viewportWidth, int viewportHeight)
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
		if (submission.renderComponent->reflectRefractMapPriority > meshPriority || submission.renderComponent == ignore) continue; // This mesh shouldn't be considered in this dynamic cube map

		// Cull objects that aren't within their respective frustums
		const AABB* aabb = submission.renderComponent->mesh->GetBoundingBox();
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

	for (RenderSubmission& submission : animatedSubmissions)
	{
		if (submission.renderComponent->reflectRefractMapPriority > meshPriority || submission.renderComponent == ignore) continue; // This mesh shouldn't be considered in this dynamic cube map

		// Cull objects that aren't within their respective frustums
		const AABB* aabb = submission.renderComponent->mesh->GetBoundingBox();
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