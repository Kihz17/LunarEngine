#include "Renderer.h"
#include "ShaderLibrary.h"
#include "EntityManager.h"
#include "Components.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <sstream>

WindowSpecs* Renderer::windowDetails = nullptr;
Framebuffer* Renderer::geometryBuffer = nullptr;
Framebuffer* Renderer::environmentBuffer = nullptr;;
Framebuffer* Renderer::cubeMapBuffer = nullptr;
Framebuffer* Renderer::irradianceBuffer = nullptr;
Framebuffer* Renderer::envPrefilterBuffer = nullptr;
Framebuffer* Renderer::envLUTBuffer = nullptr;

Texture* Renderer::envMap = nullptr;
CubeMap* Renderer::envMapCube = nullptr;
CubeMap* Renderer::envMapIrradiance = nullptr;
CubeMap* Renderer::envMapPreFilter = nullptr;

glm::mat4 Renderer::projection(1.0f);
glm::mat4 Renderer::view(1.0f);
glm::vec3 Renderer::cameraPos(0.0f);

const std::string Renderer::G_SHADER_KEY = "gShader";
const std::string Renderer::LIGHTING_SHADER_KEY = "lShader";
const std::string Renderer::CUBE_MAP_CONVERT_SHADER_KEY = "hdrToCubeShader";
const std::string Renderer::IRRADIANCE_SHADER_KEY = "cubeToIrradianceShader";
const std::string Renderer::PREFILTER_SHADER_KEY = "cubeToPrefilterShader";
const std::string Renderer::ENV_LUT_SHADER_KEY = "envLUTShader";
const std::string Renderer::CUBE_MAP_DRAW_SHADER_KEY = "cubeMapDrawShader";
const std::string Renderer::FORWARD_SHADER_KEY = "forwardShader";
const std::string Renderer::GRASS_SHADER_KEY = "grassShader";

PrimitiveShape* Renderer::quad = nullptr;
PrimitiveShape* Renderer::cube = nullptr;

static std::vector<Entity*> forwardEntites;
static std::vector<Light*> lightSources;
static Mesh* isoSphere = nullptr;

static const uint32_t MAX_GRASS_BLADES = 2048;
static VertexArrayObject* grassVAO = nullptr;
static VertexBuffer* grassVBO = nullptr;
static uint32_t grassCount = 0;
static float* grassRoots = new float[MAX_GRASS_BLADES]; // Allows for 2048 grass blades (Since this array is so big, i'm allocating on heap instead of stack)

void Renderer::Initialize(WindowSpecs* window)
{
	windowDetails = window;

	isoSphere = new Mesh("assets/models/ISO_Sphere.ply");

	Renderer::quad = new PrimitiveShape(ShapeType::Quad);
	Renderer::cube = new PrimitiveShape(ShapeType::Cube);

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
	//grassVBO->SetData(positions, 300 * sizeof(float));

	BufferLayout bufferLayout = {
		{ ShaderDataType::Float3, "vWorldPosition" }
	};
	grassVBO->SetLayout(bufferLayout);
	grassVAO->AddVertexBuffer(grassVBO);

	//------------------------
	// LOAD SHADERS
	//------------------------

	// G-Shader
	Shader* gShader = new Shader("assets/shaders/geometryBuffer.glsl");
	gShader->Bind();
	gShader->InitializeUniform("uMatModel");
	gShader->InitializeUniform("uMatModelInverseTranspose");
	gShader->InitializeUniform("uMatView");
	gShader->InitializeUniform("uMatProjection");
	gShader->InitializeUniform("uMatProjViewModel");
	gShader->InitializeUniform("uMatPrevProjViewModel");
	gShader->InitializeUniform("uAlbedoTexture1");
	gShader->InitializeUniform("uAlbedoTexture2");
	gShader->InitializeUniform("uAlbedoTexture3");
	gShader->InitializeUniform("uAlbedoTexture4");
	gShader->InitializeUniform("uAlbedoRatios");
	gShader->InitializeUniform("uColorOverride");
	gShader->InitializeUniform("uHasNormalTexture");
	gShader->InitializeUniform("uNormalTexture");
	gShader->InitializeUniform("uRoughnessTexture");
	gShader->InitializeUniform("uMetalnessTexture");
	gShader->InitializeUniform("uAmbientOcculsionTexture");
	gShader->InitializeUniform("uMaterialOverrides");

	gShader->Unbind();
	ShaderLibrary::Add(G_SHADER_KEY, gShader);

	// Lighting shader
	Shader* brdfShader = new Shader("assets/shaders/brdfLighting.glsl");
	brdfShader->Bind();
	brdfShader->InitializeUniform("uInverseView");
	brdfShader->InitializeUniform("uInverseProjection");
	brdfShader->InitializeUniform("uView");
	brdfShader->InitializeUniform("gPosition");
	brdfShader->InitializeUniform("gAlbedo");
	brdfShader->InitializeUniform("gNormal");
	brdfShader->InitializeUniform("gEffects");
	brdfShader->InitializeUniform("uLightAmount");

	for (int i = 0; i < Light::MAX_LIGHTS; i++)
	{
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].position";
			brdfShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].direction";
			brdfShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].color";
			brdfShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].param1";
			brdfShader->InitializeUniform(ss.str());
		}
	}
	
	brdfShader->InitializeUniform("uEnvMap");
	brdfShader->InitializeUniform("uIrradianceMap");
	brdfShader->InitializeUniform("uEnvMapPreFilter");
	brdfShader->InitializeUniform("uEnvMapLUT");
	brdfShader->InitializeUniform("uViewType");
	brdfShader->InitializeUniform("uReflectivity");
	brdfShader->InitializeUniform("uCameraPosition");

	brdfShader->SetInt("uViewType", 1); // Regular color view by default

	// Set samplers for lighting
	brdfShader->SetInt("gPosition", 0);
	brdfShader->SetInt("gAlbedo", 1);
	brdfShader->SetInt("gNormal", 2);
	brdfShader->SetInt("gEffects", 3);
	brdfShader->SetInt("uEnvMap", 4);
	brdfShader->SetInt("uIrradianceMap", 5);
	brdfShader->SetInt("uEnvMapPreFilter", 6);
	brdfShader->SetInt("uEnvMapLUT", 7);

	brdfShader->Unbind();
	ShaderLibrary::Add(LIGHTING_SHADER_KEY, brdfShader);

	// HDR -> Cubemap shader
	Shader* cubeMapConversionShader = new Shader("assets/shaders/equirectangularToCubeMap.glsl");
	cubeMapConversionShader->Bind();
	cubeMapConversionShader->InitializeUniform("uProjection");
	cubeMapConversionShader->InitializeUniform("uView");
	cubeMapConversionShader->InitializeUniform("uEnvMap");
	cubeMapConversionShader->Unbind();
	ShaderLibrary::Add(CUBE_MAP_CONVERT_SHADER_KEY, cubeMapConversionShader);

	// Cube map -> Irradiance map shader
	Shader* cubeMapToIrradianceShader = new Shader("assets/shaders/convoluteEnvMap.glsl");
	cubeMapToIrradianceShader->Bind();
	cubeMapToIrradianceShader->InitializeUniform("uProjection");
	cubeMapToIrradianceShader->InitializeUniform("uView");
	cubeMapToIrradianceShader->InitializeUniform("uEnvMap");
	cubeMapToIrradianceShader->Unbind();
	ShaderLibrary::Add(IRRADIANCE_SHADER_KEY, cubeMapToIrradianceShader);

	// Cubemap -> Prefilter shader
	Shader* preFilterShader = new Shader("assets/shaders/envPreFilter.glsl");
	preFilterShader->Bind();
	preFilterShader->InitializeUniform("uProjection");
	preFilterShader->InitializeUniform("uView");
	preFilterShader->InitializeUniform("uEnvMap");
	preFilterShader->InitializeUniform("uRoughness");
	preFilterShader->InitializeUniform("uCubeResolution");
	preFilterShader->Unbind();
	ShaderLibrary::Add(PREFILTER_SHADER_KEY, preFilterShader);

	// Environment Lookup Table Shader
	Shader* envLutShader = new Shader("assets/shaders/envLUT.glsl");
	envLutShader->Bind();
	envLutShader->Unbind();
	ShaderLibrary::Add(ENV_LUT_SHADER_KEY, envLutShader);

	// Cube map draw shader
	Shader* cubeMapDrawShader = new Shader("assets/shaders/environmentBuffer.glsl");
	cubeMapDrawShader->Bind();
	cubeMapDrawShader->InitializeUniform("uProjection");
	cubeMapDrawShader->InitializeUniform("uView");
	cubeMapDrawShader->InitializeUniform("uEnvMap");

	cubeMapDrawShader->SetInt("uEnvMap", 0);

	cubeMapDrawShader->Unbind();
	ShaderLibrary::Add(CUBE_MAP_DRAW_SHADER_KEY, cubeMapDrawShader);

	// Forward rendering shader
	Shader* forwardShader = new Shader("assets/shaders/forward.glsl");
	forwardShader->InitializeUniform("uMatModel");
	forwardShader->InitializeUniform("uMatView");
	forwardShader->InitializeUniform("uMatProjection");
	forwardShader->InitializeUniform("uMatModelInverseTranspose");
	forwardShader->InitializeUniform("uColorOverride");
	forwardShader->InitializeUniform("uAlbedoTexture1");
	forwardShader->InitializeUniform("uAlbedoTexture2");
	forwardShader->InitializeUniform("uAlbedoTexture3");
	forwardShader->InitializeUniform("uAlbedoTexture4");
	forwardShader->InitializeUniform("uAlbedoRatios");
	forwardShader->InitializeUniform("uHasNormalTexture");
	forwardShader->InitializeUniform("uNormalTexture");
	forwardShader->InitializeUniform("uRoughnessTexture");
	forwardShader->InitializeUniform("uMetalnessTexture");
	forwardShader->InitializeUniform("uAmbientOcculsionTexture");
	forwardShader->InitializeUniform("uMaterialOverrides");
	forwardShader->InitializeUniform("uAlphaTransparency");
	forwardShader->InitializeUniform("uIgnoreLighting");

	forwardShader->SetInt("uAlbedoTexture1", 0);
	forwardShader->SetInt("uAlbedoTexture2", 1);
	forwardShader->SetInt("uAlbedoTexture3", 2);
	forwardShader->SetInt("uAlbedoTexture4", 3);
	forwardShader->SetInt("uNormalTexture", 4);
	forwardShader->SetInt("uRoughnessTexture", 5);
	forwardShader->SetInt("uMetalnessTexture", 6);
	forwardShader->SetInt("uAmbientOcculsionTexture", 7);
	forwardShader->InitializeUniform("uLightAmount");

	for (int i = 0; i < Light::MAX_LIGHTS; i++)
	{
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].position";
			forwardShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].direction";
			forwardShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].color";
			forwardShader->InitializeUniform(ss.str());
		}
		{
			std::stringstream ss;
			ss << "uLightArray[" << i << "].param1";
			forwardShader->InitializeUniform(ss.str());
		}
	}
	ShaderLibrary::Add(FORWARD_SHADER_KEY, forwardShader);

	// Grass shader
	Shader* grassShader = new Shader("assets/shaders/grass.glsl");
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
	ShaderLibrary::Add(GRASS_SHADER_KEY, grassShader);

	//------------------------
	// SETUP FBOS
	//------------------------

	// Setup geometry buffer
	geometryBuffer = new Framebuffer();
	geometryBuffer->AddColorBuffer("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge); // Position Buffer & Depth
	geometryBuffer->AddColorBuffer("albedo", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Albedo & Roughness
	geometryBuffer->AddColorBuffer("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Normal & Metalness
	geometryBuffer->AddColorBuffer("effects", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Ambient Occulsion & Velocity
	geometryBuffer->SetRenderBuffer(windowDetails->width, windowDetails->height, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);

	environmentBuffer = new Framebuffer();
	environmentBuffer->AddColorBuffer("environment", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Linear, TextureWrapType::None);
}

void Renderer::CleanUp()
{
	delete quad;

	ShaderLibrary::CleanUp();

	delete windowDetails;
	delete geometryBuffer;

	if (cubeMapBuffer) delete cubeMapBuffer;
	if (irradianceBuffer) delete irradianceBuffer;
	if (envPrefilterBuffer) delete envPrefilterBuffer;
	if (envLUTBuffer) delete envLUTBuffer;

	if (envMap) delete envMap;
	if (envMapCube) delete envMapCube;
	if (envMapIrradiance) delete envMapIrradiance;
	if (envMapPreFilter) delete envMapPreFilter;
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

	projection = glm::perspective(camera.fov, (float)windowDetails->width / (float)windowDetails->height, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();
	cameraPos = camera.position;
}

void Renderer::EndFrame()
{
	forwardEntites.clear();
	lightSources.clear();

	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame()
{
	GeometryPass();
	EnvironmentPass();
	LightingPass();
	ForwardPass();
}

void Renderer::SetEnvironmentMapEquirectangular(const std::string& path)
{
	// Clean up previous environment variables
	if (cubeMapBuffer) delete cubeMapBuffer;
	if (irradianceBuffer) delete irradianceBuffer;
	if (envPrefilterBuffer) delete envPrefilterBuffer;
	if (envLUTBuffer) delete envLUTBuffer;

	if (envMap) delete envMap;
	if (envMapCube) delete envMapCube;
	if (envMapIrradiance) delete envMapIrradiance;
	if (envMapPreFilter) delete envMapPreFilter;

	envMap = new Texture(path, TextureFilterType::Linear, TextureWrapType::Repeat, true, true, true); // new HDR env map

	// Setup empty cube map textures
	envMapCube = new CubeMap(512, GL_LINEAR_MIPMAP_LINEAR, GL_RGB, GL_RGB16F, GL_FLOAT);
	envMapIrradiance = new CubeMap(32, GL_LINEAR, GL_RGB, GL_RGB16F, GL_FLOAT); // This can be low res like 32x32 because it is an average of surrounding pixels and linear filtering can handle most of the work
	envMapPreFilter = new CubeMap(128, GL_LINEAR_MIPMAP_LINEAR, GL_RGB, GL_RGB16F, GL_FLOAT);
	envMapPreFilter->ComputeMipmap();

	//---------------------------------------------------------------------
	// Convert the HDR equirectangular texture to its cube map equivalent
	//---------------------------------------------------------------------

	// Setup the HDR -> Cubemap framebuffer
	cubeMapBuffer = new Framebuffer();
	cubeMapBuffer->SetRenderBuffer(envMapCube->GetWidth(), envMapCube->GetHeight(), GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT);

	// Setup conversion uniforms for shader
	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 captureViews[] = // 6 different view matrices that face each side of the cube
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	const Shader* conversionShader = ShaderLibrary::Get(CUBE_MAP_CONVERT_SHADER_KEY);
	conversionShader->Bind();
	conversionShader->SetMat4("uProjection", captureProjection);
	envMap->BindToSlot(0);
	glViewport(0, 0, envMapCube->GetWidth(), envMapCube->GetHeight()); // Make sure we configure the viewport to capture the texture dimensions

	cubeMapBuffer->Bind();
	for (int i = 0; i < 6; i++) // Setup all faces of the cube
	{
		conversionShader->SetMat4("uView", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapCube->GetID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube->Draw();
	}

	envMapCube->ComputeMipmap();
	cubeMapBuffer->Unbind();

	//-------------------------------------------------------
	// Convolute the cube map to create an irradiance map
	//-------------------------------------------------------
	irradianceBuffer = new Framebuffer();
	irradianceBuffer->SetRenderBuffer(envMapIrradiance->GetWidth(), envMapIrradiance->GetHeight(), GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, false);

	const Shader* irradianceShader = ShaderLibrary::Get(IRRADIANCE_SHADER_KEY);
	irradianceShader->Bind();
	irradianceShader->SetMat4("uProjection", captureProjection);
	envMapCube->BindToSlot(0);
	glViewport(0, 0, envMapIrradiance->GetWidth(), envMapIrradiance->GetHeight()); // Make sure we configure the viewport to capture the texture dimensions

	irradianceBuffer->Bind();
	for (int i = 0; i < 6; i++) // Setup all faces of the cube
	{
		conversionShader->SetMat4("uView", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapIrradiance->GetID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube->Draw();
	}

	irradianceBuffer->Unbind();

	//---------------------------
	// Create specular IBL map
	//---------------------------
	envPrefilterBuffer = new Framebuffer();
	envPrefilterBuffer->SetRenderBuffer(envMapPreFilter->GetWidth(), envMapPreFilter->GetHeight(), GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, false);

	const Shader* preFilterShader = ShaderLibrary::Get(PREFILTER_SHADER_KEY);
	preFilterShader->SetMat4("uProjection", captureProjection);
	envMapCube->BindToSlot(0);

	envPrefilterBuffer->Bind();
	constexpr unsigned int MAX_MIP_LEVELS = 5;
	for (unsigned int i = 0; i < MAX_MIP_LEVELS; i++) // Setup the roughness mip levels
	{
		unsigned int mipWidth = envMapPreFilter->GetWidth() * std::pow(0.5, i);
		unsigned int mipHeight = envMapPreFilter->GetHeight() * std::pow(0.5, i);

		envPrefilterBuffer->UpdateRenderBufferStorage(mipWidth, mipHeight, GL_DEPTH_COMPONENT24); // Update renderbuffer dimensions to match the mip level

		glViewport(0, 0, mipWidth, mipHeight); // Make sure we configure the viewport to capture the texture dimensions
		float roughness = (float)i / (float)(MAX_MIP_LEVELS - 1); // Determin the roughness of this mip map level

		preFilterShader->SetFloat("uRoughness", roughness);
		preFilterShader->SetFloat2("uCubeResolution", glm::vec2(envMapPreFilter->GetWidth(), envMapPreFilter->GetHeight()));

		for (int i = 0; i < 6; i++) // Setup all faces of the cube
		{
			conversionShader->SetMat4("uView", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapPreFilter->GetID(), i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube->Draw();
		}
	}

	envPrefilterBuffer->Unbind();

	//------------------------------------
	// Setup environment map lookup table
	//------------------------------------
	envLUTBuffer = new Framebuffer();
	envLUTBuffer->AddColorBuffer("lut", GL_RG16F, GL_RG, GL_FLOAT, 512, 512, TextureFilterType::Linear, TextureWrapType::None);
	const ITexture* lutTexture = envLUTBuffer->GetColorBufferTexture("lut");
	envLUTBuffer->SetRenderBuffer(lutTexture->GetWidth(), lutTexture->GetHeight(), GL_DEPTH_COMPONENT24, false);
	envLUTBuffer->Bind();
	const Shader* lutShader = ShaderLibrary::Get(ENV_LUT_SHADER_KEY);

	glViewport(0, 0, lutTexture->GetWidth(), lutTexture->GetHeight()); // Make sure we configure the viewport to capture the texture dimensions
	lutShader->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	quad->Draw();
	envLUTBuffer->Unbind();

	glViewport(0, 0, windowDetails->width, windowDetails->height); // Set viewport back to native width/height
}

void Renderer::GeometryPass()
{
	glDisable(GL_BLEND); // No blend for deffered rendering
	glEnable(GL_DEPTH_TEST); // Enable depth testing for scene render

	geometryBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const Shader* gShader = ShaderLibrary::Get(G_SHADER_KEY);
	gShader->Bind();

	gShader->SetMat4("uMatProjection", projection);
	gShader->SetMat4("uMatView", view);

	const std::unordered_map<unsigned int, Entity*>& entities = EntityManager::GetEntities();
	std::unordered_map<unsigned int, Entity*>::const_iterator entityIt = entities.begin();

	while (entityIt != entities.end())
	{
		Entity* entity = entityIt->second;
		if (entity->HasComponent<Light>())
		{
			lightSources.push_back(entity->GetComponent<Light>());
		}

		if (!entity->HasComponent<Render>())  // This entity shouldn't be rendered
		{
			entityIt++;
			continue;
		}
			

		Position* positionComponent = entity->GetComponent<Position>();
		if (!positionComponent)
		{
			std::cout << "Renderable entity '" << entity->name << "' is missing position component!" << std::endl;
			entityIt++;
			continue;
		}

		Scale* scaleComponent = entity->GetComponent<Scale>();
		if (!positionComponent)
		{
			std::cout << "Renderable entity '" << entity->name << "' is missing scale component!" << std::endl;
			entityIt++;
			continue;
		}

		Rotation* rotationComponent = entity->GetComponent<Rotation>();
		if (!positionComponent)
		{
			std::cout << "Renderable entity '" << entity->name << "' is missing rotation component!" << std::endl;
			entityIt++;
			continue;
		}

		Render* renderComponent = entity->GetComponent<Render>();
		if (renderComponent->alphaTransparency < 1.0f || renderComponent->isIgnoreLighting) // Needs alpha blending, use this in the forward pass
		{
			forwardEntites.push_back(entity);
			entityIt++;
			continue;
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform *= glm::toMat4(rotationComponent->value);
		transform *= glm::scale(glm::mat4(1.0f), scaleComponent->value);
		transform *= glm::translate(glm::mat4(1.0f), positionComponent->value);

		glm::mat4 projViewModel = projection * view * transform;
		glm::mat4& prevProjViewModel = renderComponent->hasPrevProjViewModel ? renderComponent->projViewModel : projViewModel;
		renderComponent->projViewModel = projViewModel;

		gShader->SetMat4("uMatModel", transform);
		gShader->SetMat4("uMatModelInverseTranspose", glm::inverse(transform));
		gShader->SetMat4("uMatProjViewModel", projViewModel);
		gShader->SetMat4("uMatPrevProjViewModel", prevProjViewModel);

		// Color
		if (renderComponent->isColorOverride)
		{
			gShader->SetFloat4("uColorOverride", glm::vec4(renderComponent->colorOverride.x, renderComponent->colorOverride.y, renderComponent->colorOverride.z, 1.0f));
		}
		else // Bind diffuse textures
		{
			gShader->SetFloat4("uColorOverride", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

			float ratios[4];
			for (int i = 0; i < 4; i++)
			{
				if (i < renderComponent->albedoTextures.size())
				{
					renderComponent->albedoTextures[i].first->BindToSlot(i);
					gShader->SetInt(std::string("uAlbedoTexture" + std::to_string(i + 1)), i);
					ratios[i] = renderComponent->albedoTextures[i].second;
				}
				else
				{
					ratios[i] = 0.0f;
				}
			}
			gShader->SetFloat4("uAlbedoRatios", glm::vec4(ratios[0], ratios[1], ratios[2], ratios[3]));
		}

		if (renderComponent->normalTexture)
		{
			gShader->SetInt("uHasNormalTexture", GL_TRUE);
			renderComponent->normalTexture->BindToSlot(4);
			gShader->SetInt("uNormalTexture", 4);
		}
		else
		{
			gShader->SetInt("uHasNormalTexture", GL_FALSE);
		}

		if (renderComponent->HasMaterialTextures())
		{
			gShader->SetFloat4("uMaterialOverrides", glm::vec4(0.0f));

			renderComponent->roughnessTexture->BindToSlot(5);
			gShader->SetInt("uRoughnessTexture", 5);

			renderComponent->metalTexture->BindToSlot(6);
			gShader->SetInt("uMetalnessTexture", 6);

			renderComponent->aoTexture->BindToSlot(7);
			gShader->SetInt("uAmbientOcculsionTexture", 7);
		}
		else // We have no material textures
		{
			gShader->SetFloat4("uMaterialOverrides", glm::vec4(renderComponent->roughness, renderComponent->metalness, renderComponent->ao, 1.0f));
		}

		if (renderComponent->isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		renderComponent->vao->Bind();
		glDrawElements(GL_TRIANGLES, renderComponent->indexCount, GL_UNSIGNED_INT, 0);
		renderComponent->vao->Unbind();

		entityIt++;
	}

	// Grass
	const Shader* grassShader = ShaderLibrary::Get(GRASS_SHADER_KEY);
	grassShader->Bind();

	grassVAO->Bind();
	glDrawArrays(GL_POINTS, 0, 100);

	geometryBuffer->Unbind();
}

void Renderer::EnvironmentPass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Shader* environmentShader = ShaderLibrary::Get(CUBE_MAP_DRAW_SHADER_KEY);
	environmentBuffer->Bind();
	environmentShader->Bind();
	environmentShader->SetMat4("uProjection", projection);
	environmentShader->SetMat4("uView", view);
	envMapCube->BindToSlot(0);
	cube->Draw();
	environmentBuffer->Unbind();
}

void Renderer::LightingPass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Disable depth buffer so that the quad doesnt get discarded
	const Shader* lShader = ShaderLibrary::Get(LIGHTING_SHADER_KEY);
	lShader->Bind();

	// Bind G-Buffer textures
	geometryBuffer->BindColorBuffer("position", 0);
	geometryBuffer->BindColorBuffer("albedo", 1);
	geometryBuffer->BindColorBuffer("normal", 2);
	geometryBuffer->BindColorBuffer("effects", 3);

	// Bind environment map stuff
	environmentBuffer->BindColorBuffer("environment", 4);

	if (envMapIrradiance)
		envMapIrradiance->BindToSlot(5);

	if (envMapPreFilter)
		envMapPreFilter->BindToSlot(6);

	if (envLUTBuffer)
		envLUTBuffer->BindColorBuffer("lut", 7);

	lShader->SetMat4("uInverseView", glm::transpose(view));
	lShader->SetMat4("uInverseProjection", glm::inverse(projection));
	lShader->SetMat4("uView", view);
	lShader->SetFloat3("uCameraPosition", cameraPos);

	// TODO: Move these into gBuffer so it can be passed in with the geometry
	lShader->SetFloat3("uReflectivity", glm::vec3(0.04f));

	quad->Draw();
}

void Renderer::ForwardPass()
{
	geometryBuffer->BindRead(); // Bind for read only
	geometryBuffer->UnbindWrite();

	// Copy depth information from the geometry buffer -> default framebuffer
	glBlitFramebuffer(0, 0, windowDetails->width, windowDetails->height, 0, 0, windowDetails->width, windowDetails->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	geometryBuffer->Unbind();

	const Shader* forwardShader = ShaderLibrary::Get(FORWARD_SHADER_KEY);
	forwardShader->Bind();

	// Pass camera related data
	forwardShader->SetMat4("uMatView", view);
	forwardShader->SetMat4("uMatProjection", projection);

	// Draw geometry
	for (Entity* entity : forwardEntites)
	{
		Position* positionComponent = entity->GetComponent<Position>();
		Scale* scaleComponent = entity->GetComponent<Scale>();
		Rotation* rotationComponent = entity->GetComponent<Rotation>();
		Render* renderComponent = entity->GetComponent<Render>();

		glm::mat4 transform = glm::mat4(1.0f);
		transform *= glm::toMat4(rotationComponent->value);
		transform *= glm::scale(glm::mat4(1.0f), scaleComponent->value);
		transform *= glm::translate(glm::mat4(1.0f), positionComponent->value);

		forwardShader->SetMat4("uMatModel", transform);
		forwardShader->SetMat4("uMatModelInverseTranspose", glm::inverse(transform));

		// Color
		if (renderComponent->isColorOverride)
		{
			forwardShader->SetFloat4("uColorOverride", glm::vec4(renderComponent->colorOverride.x, renderComponent->colorOverride.y, renderComponent->colorOverride.z, 1.0f));
		}
		else // Bind diffuse textures
		{
			forwardShader->SetFloat4("uColorOverride", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

			float ratios[4];
			for (int i = 0; i < 4; i++)
			{
				if (i < renderComponent->albedoTextures.size())
				{
					renderComponent->albedoTextures[i].first->BindToSlot(i);
					forwardShader->SetInt(std::string("uAlbedoTexture" + std::to_string(i + 1)), i);
					ratios[i] = renderComponent->albedoTextures[i].second;
				}
				else
				{
					ratios[i] = 0.0f;
				}
			}
			forwardShader->SetFloat4("uAlbedoRatios", glm::vec4(ratios[0], ratios[1], ratios[2], ratios[3]));
		}

		// Normal
		if (renderComponent->normalTexture)
		{
			forwardShader->SetInt("uHasNormalTexture", GL_TRUE);
			renderComponent->normalTexture->BindToSlot(4);
			forwardShader->SetInt("uNormalTexture", 4);
		}
		else
		{
			forwardShader->SetInt("uHasNormalTexture", GL_FALSE);
		}

		// Materials
		if (renderComponent->HasMaterialTextures())
		{
			forwardShader->SetFloat4("uMaterialOverrides", glm::vec4(0.0f));

			renderComponent->roughnessTexture->BindToSlot(5);
			forwardShader->SetInt("uRoughnessTexture", 5);

			renderComponent->metalTexture->BindToSlot(6);
			forwardShader->SetInt("uMetalnessTexture", 6);

			renderComponent->aoTexture->BindToSlot(7);
			forwardShader->SetInt("uAmbientOcculsionTexture", 7);
		}
		else // We have no material textures
		{
			forwardShader->SetFloat4("uMaterialOverrides", glm::vec4(renderComponent->roughness, renderComponent->metalness, renderComponent->ao, 1.0f));
		}

		forwardShader->SetInt("uIgnoreLighting", renderComponent->isIgnoreLighting ? GL_TRUE : GL_FALSE);

		forwardShader->SetFloat("uAlphaTransparency", renderComponent->alphaTransparency);

		if (renderComponent->isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		renderComponent->vao->Bind();
		glDrawElements(GL_TRIANGLES, renderComponent->indexCount, GL_UNSIGNED_INT, 0);
		renderComponent->vao->Unbind();
	}

	// Draw lights
	for (Light* light : lightSources)
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform *= glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		transform *= glm::translate(glm::mat4(1.0f), light->GetPosition());

		forwardShader->SetMat4("uMatModel", transform);
		forwardShader->SetMat4("uMatModelInverseTranspose", glm::inverse(transform));

		forwardShader->SetFloat4("uColorOverride", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		forwardShader->SetInt("uIgnoreLighting", GL_TRUE);
		forwardShader->SetFloat("uAlphaTransparency", 1.0f);

		isoSphere->GetVertexArray()->Bind();
		glDrawElements(GL_TRIANGLES, isoSphere->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
		isoSphere->GetVertexArray()->Unbind();
	}
}