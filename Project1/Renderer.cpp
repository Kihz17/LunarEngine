#include "Renderer.h"
#include "ShaderLibrary.h"

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

std::unordered_map<std::string, SubmittedGeometry> Renderer::defferedGeometry;
//std::unordered_map<std::string, glm::mat4> Renderer::prevProjViewModels;

std::vector<ForwardGeometry> Renderer::forwardGeometry;

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

PrimitiveShape* Renderer::quad = nullptr;
PrimitiveShape* Renderer::cube = nullptr;

std::unordered_map<std::string, Light*> Renderer::lights;

void Renderer::Initialize(WindowSpecs* window)
{
	windowDetails = window;

	Renderer::quad = new PrimitiveShape(ShapeType::Quad);
	Renderer::cube = new PrimitiveShape(ShapeType::Cube);

	//------------------------
	// LOAD SHADERS
	//------------------------

	// G-Shader
	Shader* gShader = new Shader("assets/shaders/geometryBuffer.glsl");
	gShader->Bind();
	gShader->InitializeUniform("uMatModel");
	gShader->InitializeUniform("uMatView");
	gShader->InitializeUniform("uMatProjection");
	gShader->InitializeUniform("uMatProjViewModel");
	gShader->InitializeUniform("uMatPrevProjViewModel");
	gShader->InitializeUniform("uAlbedoTexture1");
	gShader->InitializeUniform("uAlbedoTexture2");
	gShader->InitializeUniform("uAlbedoTexture3");
	gShader->InitializeUniform("uAlbedoTexture4");
	gShader->InitializeUniform("uAlbedoRatios");
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
	brdfShader->InitializeUniform("gViewPosition");
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
	brdfShader->SetInt("gViewPosition", 4);
	brdfShader->SetInt("uEnvMap", 5);
	brdfShader->SetInt("uIrradianceMap", 6);
	brdfShader->SetInt("uEnvMapPreFilter", 7);
	brdfShader->SetInt("uEnvMapLUT", 8);

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
	forwardShader->InitializeUniform("uDiffuse1");
	forwardShader->InitializeUniform("uDiffuse2");
	forwardShader->InitializeUniform("uDiffuse3");
	forwardShader->InitializeUniform("uDiffuse4");
	forwardShader->InitializeUniform("uDiffuseRatios");
	forwardShader->InitializeUniform("uAlphaTransparency");
	ShaderLibrary::Add(FORWARD_SHADER_KEY, forwardShader);
	//------------------------
	// SETUP FBOS
	//------------------------

	// Setup geometry buffer
	geometryBuffer = new Framebuffer();
	geometryBuffer->AddColorBuffer("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge); // Position Buffer & Depth
	geometryBuffer->AddColorBuffer("albedo", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Albedo & Roughness
	geometryBuffer->AddColorBuffer("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Normal & Metalness
	geometryBuffer->AddColorBuffer("effects", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Ambient Occulsion & Velocity
	geometryBuffer->AddColorBuffer("viewPosition", GL_RGB16F, GL_RGB, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge); // View Position Buffer
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
	if (type <= 0 || type >= 10) return; // Out of range

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
	forwardGeometry.clear();
	 
	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame(float deltaTime)
{
	//------------------------
	// GEOMETRY RENDER PASS
	//------------------------
	glDisable(GL_BLEND); // No blend for deffered rendering
	glEnable(GL_DEPTH_TEST); // Enable depth testing for scene render

	geometryBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const Shader* gShader = ShaderLibrary::Get(G_SHADER_KEY);
	gShader->Bind();

	gShader->SetMat4("uMatProjection", projection);
	gShader->SetMat4("uMatView", view);

	std::unordered_map<std::string, SubmittedGeometry>::iterator defferedIt = defferedGeometry.begin();
	while (defferedIt != defferedGeometry.end())
	{
		SubmittedGeometry& geometry = defferedIt->second;

		glm::mat4 transform = glm::mat4(1.0f);
		transform *= glm::rotate(glm::mat4(1.0f), geometry.orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), geometry.orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), geometry.orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		transform *= glm::scale(glm::mat4(1.0f), geometry.scale);
		transform *= glm::translate(glm::mat4(1.0f), geometry.position);

		glm::mat4 projViewModel = projection * view * transform;
		glm::mat4& prevProjViewModel = geometry.hasPrevProjViewModel ? geometry.projViewModel : projViewModel;
		geometry.projViewModel = projViewModel;

		gShader->SetMat4("uMatModel", transform);
		gShader->SetMat4("uMatProjViewModel", projViewModel);
		gShader->SetMat4("uMatPrevProjViewModel", prevProjViewModel);

		// Bind albedo textures
		float ratios[4];
		for (int i = 0; i < 4; i++)
		{
			if (i < geometry.albedoTextures.size())
			{
				geometry.albedoTextures[i].first->BindToSlot(i);
				gShader->SetInt(std::string("uAlbedoTexture" + std::to_string(i + 1)), i);
				ratios[i] = geometry.albedoTextures[i].second;
			}
			else
			{
				ratios[i] = 0.0f;
			}
		}
		gShader->SetFloat4("uAlbedoRatios", glm::vec4(ratios[0], ratios[1], ratios[2], ratios[3]));

		geometry.normalTexture->BindToSlot(4);
		gShader->SetInt("uNormalTexture", 4);

		if (geometry.hasMaterialTextures)
		{
			gShader->SetFloat4("uMaterialOverrides", glm::vec4(0.0f));

			geometry.roughnessTexture->BindToSlot(5);
			gShader->SetInt("uRoughnessTexture", 5);

			geometry.metalTexture->BindToSlot(6);
			gShader->SetInt("uMetalnessTexture", 6);

			geometry.aoTexture->BindToSlot(7);
			gShader->SetInt("uAmbientOcculsionTexture", 7);
		}
		else // We have no material textures
		{
			gShader->SetFloat4("uMaterialOverrides", glm::vec4(geometry.roughness, geometry.metalness, geometry.ao, 1.0f));
		}

		if (geometry.isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometry.vao->Bind();
		glDrawElements(GL_TRIANGLES, geometry.indexCount, GL_UNSIGNED_INT, 0);
		geometry.vao->Unbind();

		defferedIt++;
	}

	gShader->Unbind();
	geometryBuffer->Unbind();

	//------------------------
	// ENVIRONMENT PASS
	//------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Shader* environmentShader = ShaderLibrary::Get(CUBE_MAP_DRAW_SHADER_KEY);
	environmentBuffer->Bind();
	environmentShader->Bind();
	environmentShader->SetMat4("uProjection", projection);
	environmentShader->SetMat4("uView", view);
	envMapCube->BindToSlot(0);
	cube->Draw();
	environmentBuffer->Unbind();

	//------------------------
	// LIGHTING RENDER PASS
	//------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Disable depth buffer so that the quad doesnt get discarded
	const Shader* lShader = ShaderLibrary::Get(LIGHTING_SHADER_KEY);
	lShader->Bind();

	// Bind G-Buffer textures
	geometryBuffer->BindColorBuffer("position", 0);
	geometryBuffer->BindColorBuffer("albedo", 1);
	geometryBuffer->BindColorBuffer("normal", 2);
	geometryBuffer->BindColorBuffer("effects", 3);
	geometryBuffer->BindColorBuffer("viewPosition", 4);

	// Bind environment map stuff
	environmentBuffer->BindColorBuffer("environment", 5);

	if (envMapIrradiance)
		envMapIrradiance->BindToSlot(6);

	if (envMapPreFilter)
		envMapPreFilter->BindToSlot(7);

	if (envLUTBuffer)
		envLUTBuffer->BindColorBuffer("lut", 8);

	lShader->SetMat4("uInverseView", glm::transpose(view));
	lShader->SetMat4("uInverseProjection", glm::inverse(view));
	lShader->SetMat4("uView", view);
	lShader->SetFloat3("uCameraPosition", cameraPos);

	// TODO: Move these into gBuffer so it can be passed in with the geometry
	lShader->SetFloat3("uReflectivity", glm::vec3(0.04f)); 

	quad->Draw();
	lShader->Unbind();

	//------------------------
	// FORWARD RENDER PASS
	//------------------------
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
	for (ForwardGeometry& geometry : forwardGeometry)
	{
		forwardShader->SetMat4("uMatModel", geometry.transform);
		forwardShader->SetMat4("uMatModelInverseTranspose", glm::inverse(geometry.transform));

		if (geometry.isColorOverride)
		{
			forwardShader->SetFloat4("uColorOverride", glm::vec4(geometry.colorOverride.x, geometry.colorOverride.y, geometry.colorOverride.z, 1.0f));
		}
		else // Bind diffuse textures
		{
			float ratios[4];
			for (int i = 0; i < 4; i++)
			{
				if (i < geometry.diffuseTextures.size())
				{
					geometry.diffuseTextures[i].first->BindToSlot(i);
					forwardShader->SetInt(std::string("uDiffuseTexture" + std::to_string(i + 1)), i);
					ratios[i] = geometry.diffuseTextures[i].second;
				}
				else
				{
					ratios[i] = 0.0f;
				}
			}
			forwardShader->SetFloat4("uDiffuseRatios", glm::vec4(ratios[0], ratios[1], ratios[2], ratios[3]));
		}
		
		forwardShader->SetFloat("uAlphaTransparency", geometry.alphaTransparency);

		if (geometry.isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometry.vao->Bind();
		glDrawElements(GL_TRIANGLES, geometry.indexCount, GL_UNSIGNED_INT, 0);
		geometry.vao->Unbind();
	}
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

Light* Renderer::AddLight(const std::string& name, const LightInfo& lightInfo)
{
	if (lights.find(name) != Renderer::lights.end())
	{
		std::cout << "Light with name '" << name << "' already exists!" << std::endl;
		return nullptr;
	}

	Light* light = new Light(lightInfo.postion, lightInfo.direction, lightInfo.color, lightInfo.lightType, lightInfo.radius, lightInfo.attenMode, lightInfo.on);
	Renderer::lights.insert({ name, light });
	return light;
}