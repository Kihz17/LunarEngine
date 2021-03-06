#pragma once

#include "Window.h"
#include "RenderSubmission.h"
#include "Camera.h"
#include "PrimitiveShape.h"
#include "Frustum.h"
#include "CubeMap.h"
#include "Texture3D.h"
#include "SimpleFastVector.h"

#include "GeometryPass.h"
#include "EnvironmentMapPass.h"
#include "LightingPass.h"
#include "ForwardRenderPass.h"
#include "CascadedShadowMapping.h"
#include "LinePass.h"
#include "DynamicCubeMapRenderer.h"
#include "CloudPass.h"
#include "Frustum.h"
#include "GrassPass.h"
#include "TerrainPass.h"
#include "TerrainGenerationInfo.h"
#include "GrassCluster.h"
#include "ProceduralGrassPass.h"

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

	static CubeMap* GenerateDynamicCubeMap(const glm::vec3& center, ReflectRefractMapPriorityType meshPriority, RenderComponent* ignore, int viewportWidth = windowDetails->width, int viewportHeight = windowDetails->height);

	static Light* GetMainLightSource() { return mainLight; }
	static void SetMainLightSource(Light* light) { mainLight = light; };

	static const Frustum& GetViewFrustum() { return viewFrustum; }

	static TerrainGenerationInfo& GetTerrainInfo() { return terrainInfo; }
	static std::vector<GrassCluster>& GetGrassClusters() { return grassClusters; }

	static CloudPass* GetCloudPass() { return cloudPass; }

	static const std::string LIGHTING_SHADER_KEY;
	static const std::string FORWARD_SHADER_KEY;

	static PrimitiveShape* quad;
	static PrimitiveShape* cube;

	static CubeMap* envMap1;
	static CubeMap* envMap2;
	static glm::vec4 environmentMixFactors;

private:
	friend class GameEngine;

	const static WindowSpecs* windowDetails;

	static std::vector<RenderSubmission> culledShadowSubmissions;
	static std::vector<RenderSubmission> culledSubmissions;
	static std::vector<RenderSubmission> culledAnimatedSubmissions;
	static std::vector<RenderSubmission> culledAnimatedShadowSubmissions;
	static std::vector<RenderSubmission> culledForwardSubmissions;
	static std::vector<LineRenderSubmission> lineSubmissions;

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
	static GrassPass* grassPass;
	static TerrainPass* terrainPass;

	static TerrainGenerationInfo terrainInfo;
	static std::vector<GrassCluster> grassClusters;

	static DynamicCubeMapRenderer* dynamicCubeMapGenerator;

	static Light* mainLight;
};