#pragma once

#include "Window.h"
#include "RenderSubmission.h"
#include "Camera.h"
#include "PrimitiveShape.h"

#include "GeometryPass.h"
#include "EnvironmentMapPass.h"
#include "LightingPass.h"
#include "ForwardRenderPass.h"

#include <vector>

class Renderer
{
public:
	static void Initialize(WindowSpecs* window);
	static void CleanUp();

	static void SetViewType(uint32_t type);

	static void SetEnvironmentMapEquirectangular(const std::string& path);

	static void DrawFrame();

	static void BeginFrame(const Camera& camera);
	static void EndFrame();

	static void Submit(const RenderSubmission& submission);

	static const std::string LIGHTING_SHADER_KEY;
	static const std::string FORWARD_SHADER_KEY;
private:
	const static WindowSpecs* windowDetails;

	static glm::mat4 projection;
	static glm::mat4 view;
	static glm::vec3 cameraPos;

	static float farPlane;
	static float nearPlane;

	// Render Pass Objects
	static GeometryPass* geometryPass;
	static EnvironmentMapPass* envMapPass;
	static LightingPass* lightingPass;
	static ForwardRenderPass* forwardPass;
};