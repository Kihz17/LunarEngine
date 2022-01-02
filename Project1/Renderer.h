#pragma once

#include "Framebuffer.h"
#include "Mesh.h"
#include "Camera.h"
#include "Light.h"
#include "PrimitiveShape.h"

#include <vector>

struct SubmittedGeometry
{
	std::string handle;
	Mesh* mesh;
	glm::mat4 transform;

	std::vector<Texture*> albedoTextures;
	Texture* normalTexture;
	Texture* roughnessTexture;
	Texture* metalTexture;
	Texture* aoTexture;
};

struct WindowSpecs
{
	GLFWwindow* window;
	int width, height;
};
class Renderer
{
public:
	static void Initialize(WindowSpecs* window);
	static void CleanUp();

	static void SetViewType(uint32_t type);

	static void SubmitMesh(const SubmittedGeometry& geometry);
	static void SubmitMesh(const std::string& handle, Mesh* mesh, const glm::mat4& transform, std::vector<Texture*> albedoTextures = std::vector<Texture*>(), 
		Texture* normalTexture = nullptr, Texture* roughnessTexture = nullptr, Texture* metalTexture = nullptr, Texture* aoTexture = nullptr);

	static void DrawFrame(float deltaTime);

	static void BeginFrame(const Camera& camera);
	static void EndFrame();

	static const std::string G_SHADER_KEY;
	static const std::string LIGHTING_SHADER_KEY;

private:
	static WindowSpecs* windowDetails;

	static Framebuffer* geometryBuffer;
	
	// Environment map stuff
	static Texture* envMap;
	static Texture* envMapIrradiance;
	static Texture* envMapPreFilter;
	static Texture* envMapLUT;

	static std::vector<SubmittedGeometry> defferedGeometry;
	static std::unordered_map<std::string, glm::mat4> prevProjViewModels;

	static std::vector<SubmittedGeometry> forwardGeometry;

	static glm::mat4 projection;
	static glm::mat4 view;

	static PrimitiveShape* quad;
};