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
	VertexArrayObject* vao;
	uint32_t indexCount = 0;

	glm::vec3 orientation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 position = glm::vec3(0.0f);

	bool hasPrevProjViewModel = false;
	glm::mat4 projViewModel;

	std::vector<std::pair<Texture*, float>> albedoTextures;
	Texture* normalTexture;
	Texture* roughnessTexture;
	Texture* metalTexture;
	Texture* aoTexture;

	bool hasMaterialTextures = true;
	float roughness = 0.01f;
	float metalness = 0.02f;
	float ao = 1.0f;

	bool isWireframe = false;
};

struct ForwardGeometry
{
	std::string handle;
	VertexArrayObject* vao;
	uint32_t indexCount = 0;
	glm::mat4 transform;

	std::vector<std::pair<Texture*, float>> diffuseTextures;
	float alphaTransparency = 1.0f;

	bool isColorOverride = false;
	glm::vec3 colorOverride;

	bool isWireframe = false;
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

	static void SetEnvironmentMapEquirectangular(const std::string& path);

	static void DrawFrame(float deltaTime);

	static void BeginFrame(const Camera& camera);
	static void EndFrame();

	static Light* AddLight(const std::string& name, const LightInfo& lightInfo);
	static std::unordered_map<std::string, Light*> GetLights() { return lights; }

	static void SubmitForwardGeometry(const ForwardGeometry& geometry) { forwardGeometry.push_back(geometry); }

	static void SubmitDefferedGeometry(const SubmittedGeometry& geometry) { defferedGeometry.insert({ geometry.handle, geometry }); }
	static bool ContainsDefferedGeometry(const std::string& handle) { return defferedGeometry.count(handle) > 0; }
	static void RemoveDefferedGeometry(const std::string& handle) { defferedGeometry.erase(handle); }
	static std::unordered_map<std::string, SubmittedGeometry>& GetDefferedGeometry() { return defferedGeometry; }
	static SubmittedGeometry& GetDefferedGeometry(const std::string& handle)
	{
		std::unordered_map<std::string, SubmittedGeometry>::iterator it = defferedGeometry.find(handle);
		if (it == defferedGeometry.end())
		{
			std::cout << "Could not find Deffered Geometry with handle '" << handle << "'!" << std::endl;
		}

		return it->second;
	}

	static const std::string G_SHADER_KEY;
	static const std::string LIGHTING_SHADER_KEY;
	static const std::string CUBE_MAP_CONVERT_SHADER_KEY;
	static const std::string IRRADIANCE_SHADER_KEY;
	static const std::string PREFILTER_SHADER_KEY;
	static const std::string ENV_LUT_SHADER_KEY;
	static const std::string CUBE_MAP_DRAW_SHADER_KEY;
	static const std::string FORWARD_SHADER_KEY;

private:
	static WindowSpecs* windowDetails;

	static Framebuffer* geometryBuffer;
	static Framebuffer* environmentBuffer;
	static Framebuffer* cubeMapBuffer;
	static Framebuffer* irradianceBuffer;
	static Framebuffer* envPrefilterBuffer;
	static Framebuffer* envLUTBuffer;

	// Environment map stuff
	static Texture* envMap;
	static CubeMap* envMapCube;
	static CubeMap* envMapIrradiance;
	static CubeMap* envMapPreFilter;

	static std::unordered_map<std::string, SubmittedGeometry> defferedGeometry;

	static std::vector<ForwardGeometry> forwardGeometry;

	static glm::mat4 projection;
	static glm::mat4 view;
	static glm::vec3 cameraPos;

	static PrimitiveShape* quad;
	static PrimitiveShape* cube;

	static std::unordered_map<std::string, Light*> lights;
};