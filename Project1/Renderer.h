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

	glm::vec3* orientation = nullptr;
	glm::vec3* scale = nullptr;
	glm::vec3* position = nullptr;

	bool hasPrevProjViewModel = false;
	glm::mat4 projViewModel;

	std::vector<std::pair<Texture*, float>> albedoTextures;
	Texture* normalTexture = nullptr;
	Texture* roughnessTexture = nullptr;
	Texture* metalTexture = nullptr;
	Texture* aoTexture = nullptr;

	float roughness = 0.01f;
	float metalness = 0.02f;
	float ao = 1.0f;

	bool isWireframe = false;
	bool isIgnoreLighting = false;

	bool Validate()
	{
		return vao && orientation && scale && position;
	}

	bool HasMaterialTextures()
	{
		return roughnessTexture && metalTexture && aoTexture;
	}
};

struct ForwardGeometry
{
	std::string handle;
	VertexArrayObject* vao;
	uint32_t indexCount = 0;

	glm::vec3* orientation = nullptr;
	glm::vec3* scale = nullptr;
	glm::vec3* position = nullptr;

	bool isColorOverride = false;
	glm::vec3 colorOverride = glm::vec3(0.0f);
	std::vector<std::pair<Texture*, float>> albedoTextures;

	Texture* normalTexture = nullptr;
	Texture* roughnessTexture = nullptr;
	Texture* metalTexture = nullptr;
	Texture* aoTexture = nullptr;

	float roughness = 0.01f;
	float metalness = 0.02f;
	float ao = 1.0f;

	bool isWireframe = false;
	bool isIgnoreLighting = false;

	float alphaTransparency = 1.0f;

	bool Validate()
	{
		return vao && orientation && scale && position;
	}

	bool HasMaterialTextures()
	{
		return roughnessTexture && metalTexture && aoTexture;
	}
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

	static void SubmitForwardGeometry(const ForwardGeometry& geometry) { forwardGeometry.insert({ geometry.handle, geometry }); } 	// TODO: SORT BASED ON TRANSPARENCY 
	static bool ContainsForwardGeometry(const std::string& handle) { return forwardGeometry.count(handle) > 0; }
	static void RemoveForwardGeometry(const std::string& handle) { forwardGeometry.erase(handle); }
	static std::unordered_map<std::string, ForwardGeometry>& GetForwardGeometry() { return forwardGeometry; }
	static ForwardGeometry& GetForwardGeometry(const std::string& handle)
	{
		std::unordered_map<std::string, ForwardGeometry>::iterator it = forwardGeometry.find(handle);
		if (it == forwardGeometry.end())
		{
			std::cout << "Could not find Forward Geometry with handle '" << handle << "'!" << std::endl;
		}

		return it->second;
	}

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
	static std::unordered_map<std::string, ForwardGeometry> forwardGeometry;

	static glm::mat4 projection;
	static glm::mat4 view;
	static glm::vec3 cameraPos;

	static PrimitiveShape* quad;
	static PrimitiveShape* cube;

	static std::unordered_map<std::string, Light*> lights;
};