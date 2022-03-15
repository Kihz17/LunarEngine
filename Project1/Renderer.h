#pragma once

#include "Window.h"
#include "RenderSubmission.h"
#include "Camera.h"
#include "PrimitiveShape.h"
#include "Frustum.h"
#include "CubeMap.h"
#include "Texture3D.h"

#include "GeometryPass.h"
#include "EnvironmentMapPass.h"
#include "LightingPass.h"
#include "ForwardRenderPass.h"
#include "CascadedShadowMapping.h"
#include "LinePass.h"
#include "DynamicCubeMapRenderer.h"
#include "CloudPass.h"

#include <vector>
#include <unordered_map>

class CubeMap;
class Light;
class Renderer
{
public:
	static void Initialize(const Camera& camera, WindowSpecs* window);
	static void CleanUp();

	static void SetViewType(uint32_t type);

	static void DrawFrame();

	static void BeginFrame(const Camera& camera);
	static void EndFrame();

	static void Submit(const RenderSubmission& submission);
	static void SubmitLines(const LineRenderSubmission& submission);

	static void SetShadowMappingDirectionalLight(Light* light);

	static CubeMap* GenerateDynamicCubeMap(const glm::vec3& center, ReflectRefractMapPriorityType meshPriority, RenderComponent* ignore, int viewportWidth = windowDetails->width, int viewportHeight = windowDetails->height);

	static void SetEnvironmentMap(CubeMap* cm) { envMap = cm; }
	static CubeMap* GetEnvironmentMap() { return envMap; }

	static const std::string LIGHTING_SHADER_KEY;
	static const std::string FORWARD_SHADER_KEY;

	static PrimitiveShape* quad;
	static PrimitiveShape* cube;

private:
	const static WindowSpecs* windowDetails;

	static glm::mat4 projection;
	static glm::mat4 view;
	static glm::vec3 cameraPos;
	static Frustum viewFrustum;
	static float shadowCullRadius;

	static float farPlane;
	static float nearPlane;

	// Render Pass Objects
	static GeometryPass* geometryPass;
	static EnvironmentMapPass* envMapPass;
	static LightingPass* lightingPass;
	static ForwardRenderPass* forwardPass;
	static CascadedShadowMapping* shadowMappingPass;
	static LinePass* linePass;
	static CloudPass* cloudPass;

	static CubeMap* envMap;
	static DynamicCubeMapRenderer* dynamicCubeMapGenerator;
};