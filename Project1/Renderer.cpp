#include "Renderer.h"
#include "ShaderLibrary.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <sstream>

WindowSpecs* Renderer::windowDetails = nullptr;
Framebuffer* Renderer::geometryBuffer = nullptr;

Texture* Renderer::envMap = nullptr;
Texture* Renderer::envMapIrradiance = nullptr;
Texture* Renderer::envMapPreFilter = nullptr;
Texture* Renderer::envMapLUT = nullptr;

std::vector<SubmittedGeometry> Renderer::defferedGeometry;
std::unordered_map<std::string, glm::mat4> Renderer::prevProjViewModels;

std::vector<SubmittedGeometry> Renderer::forwardGeometry;

glm::mat4 Renderer::projection(1.0f);
glm::mat4 Renderer::view(1.0f);

const std::string Renderer::G_SHADER_KEY = "gShader";
const std::string Renderer::LIGHTING_SHADER_KEY = "lShader";

PrimitiveShape* Renderer::quad = nullptr;

void Renderer::Initialize(WindowSpecs* window)
{
	windowDetails = window;

	Renderer::quad = new PrimitiveShape(ShapeType::Quad);

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
	gShader->InitializeUniform("uAlbedoColor");
	gShader->InitializeUniform("uAlbedoTexture1");
	gShader->InitializeUniform("uAlbedoTexture2");
	gShader->InitializeUniform("uAlbedoTexture3");
	gShader->InitializeUniform("uAlbedoTexture4");
	gShader->InitializeUniform("uNormalTexture");
	gShader->InitializeUniform("uRoughnessTexture");
	gShader->InitializeUniform("uMetalnessTexture");
	gShader->InitializeUniform("uAmbientOcculsionTexture");
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

	gShader->Unbind();
	ShaderLibrary::Add(LIGHTING_SHADER_KEY, brdfShader);

	//------------------------
	// SETUP FBOS
	//------------------------

	// Setup geometry buffer
	geometryBuffer = new Framebuffer();
	geometryBuffer->AddColorBuffer("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::ClampToEdge); // Position Buffer
	geometryBuffer->AddColorBuffer("albedo", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Albedo & Roughness
	geometryBuffer->AddColorBuffer("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Normal & Metalness
	geometryBuffer->AddColorBuffer("effects", GL_RGBA16F, GL_RGBA, GL_FLOAT, windowDetails->width, windowDetails->height, TextureFilterType::Nearest, TextureWrapType::None); // Ambient Occulsion & Velocity
	geometryBuffer->SetRenderBuffer(windowDetails->width, windowDetails->height, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
}

void Renderer::CleanUp()
{
	delete quad;

	ShaderLibrary::CleanUp();

	delete windowDetails;
	delete geometryBuffer;
	delete envMap;
	delete envMapIrradiance;
	delete envMapPreFilter;
	delete envMapLUT;
}

void Renderer::SetViewType(uint32_t type)
{
	if (type <= 0 || type >= 9) return; // Out of range

	const Shader* shader = ShaderLibrary::Get(LIGHTING_SHADER_KEY);
	shader->Bind();
	shader->SetInt("uViewType", type);
	shader->Unbind();
}

void Renderer::SubmitMesh(const SubmittedGeometry& geometry)
{
	defferedGeometry.push_back(geometry);
}

void Renderer::SubmitMesh(const std::string& handle, Mesh* mesh, const glm::mat4& transform, std::vector<Texture*> albedoTextures, Texture* normalTexture, Texture* roughnessTexture, Texture* metalTexture, Texture* aoTexture)
{
	SubmittedGeometry geometry;
	geometry.handle = handle;
	geometry.mesh = mesh;
	geometry.transform = transform;
	geometry.albedoTextures = albedoTextures;
	geometry.normalTexture = normalTexture;
	geometry.roughnessTexture = roughnessTexture;
	geometry.metalTexture = metalTexture;
	geometry.aoTexture = aoTexture;

	defferedGeometry.push_back(geometry);
}

void Renderer::BeginFrame(const Camera& camera)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const Shader* gShader = ShaderLibrary::Get(G_SHADER_KEY);
	gShader->Bind();

	projection = glm::perspective(camera.fov, (float)windowDetails->width / (float)windowDetails->height, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();

	gShader->SetMat4("uMatProjection", projection);
	gShader->SetMat4("uMatView", view);
	gShader->SetFloat3("uAlbedoColor", glm::vec3(1.0f));
}

void Renderer::EndFrame()
{
	defferedGeometry.clear();
	forwardGeometry.clear();

	glfwSwapBuffers(windowDetails->window);
}

void Renderer::DrawFrame(float deltaTime)
{
	//------------------------
	// GEOMETRY RENDER PASS
	//------------------------
	glDisable(GL_BLEND); // No blend for deffered rendering

	geometryBuffer->Bind();

	const Shader* gShader = ShaderLibrary::Get(G_SHADER_KEY);
	gShader->Bind();

	for (SubmittedGeometry& geometry : defferedGeometry)
	{
		glm::mat4 projViewModel = projection * view * geometry.transform;
		glm::mat4& prevProjViewModel = prevProjViewModels.count(geometry.handle) <= 0 ? projViewModel : prevProjViewModels.at(geometry.handle);
		prevProjViewModels.insert({ geometry.handle, projViewModel });

		gShader->SetMat4("uMatModel", geometry.transform);
		gShader->SetMat4("uMatProjViewModel", projViewModel);
		gShader->SetMat4("uMatPrevProjViewModel", prevProjViewModel);

		// Bind albedo textures
		for (int i = 0; i < std::min((int) geometry.albedoTextures.size(), 4); i++)
		{
			Texture* albedo = geometry.albedoTextures[i];
			albedo->BindToSlot(i);
			gShader->SetInt(std::string("uAlbedoTexture" + std::to_string(i + 1)), i);
		}

		if (geometry.normalTexture)
		{
			geometry.normalTexture->BindToSlot(4);
			gShader->SetInt("uNormalTexture", 4);
		}
			
		if (geometry.roughnessTexture)
		{
			geometry.roughnessTexture->BindToSlot(5);
			gShader->SetInt("uRoughnessTexture", 5);
		}
			
		if (geometry.metalTexture)
		{
			geometry.metalTexture->BindToSlot(6);
			gShader->SetInt("uMetalnessTexture", 6);
		}

		if (geometry.aoTexture)
		{
			geometry.aoTexture->BindToSlot(7);
			gShader->SetInt("uAmbientOcculsionTexture", 7);
		}
			
		geometry.mesh->GetVertexArray()->Bind();
		glDrawElements(GL_TRIANGLES, geometry.mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
		geometry.mesh->GetVertexArray()->Unbind();
	}

	geometryBuffer->Unbind();

	//------------------------
	// LIGHTING RENDER PASS
	//------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Shader* lShader = ShaderLibrary::Get(LIGHTING_SHADER_KEY);

	lShader->Bind();

	// Bind G-Buffer textures
	geometryBuffer->BindColorBuffer("position", 0);
	geometryBuffer->BindColorBuffer("albedo", 1);
	geometryBuffer->BindColorBuffer("normal", 2);
	geometryBuffer->BindColorBuffer("effects", 3);
	
	// Bind environment map stuff
	if (envMap)
		envMap->BindToSlot(4);

	if (envMapIrradiance)
		envMapIrradiance->BindToSlot(5);

	if (envMapPreFilter)
		envMapPreFilter->BindToSlot(6);

	if (envMapLUT)
		envMapLUT->BindToSlot(7);

	lShader->SetMat4("uInverseView", glm::transpose(view));
	lShader->SetMat4("uInverseProjection", glm::inverse(view));
	lShader->SetMat4("uView", view);

	// TODO: Move these into gBuffer so it can be passed in with the geometry
	lShader->SetFloat3("uReflectivity", glm::vec3(0.04f)); 

	quad->Draw();

	lShader->Unbind();

	//------------------------
	// FORWARD RENDER PASS
	//------------------------
	glEnable(GL_BLEND); // Re-enable blend for forward rendering
}