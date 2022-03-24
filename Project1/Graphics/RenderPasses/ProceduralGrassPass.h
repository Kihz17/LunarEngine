#pragma once

#include "IFrameBuffer.h"
#include "GrassCluster.h"
#include "Shader.h"
#include "TerrainGenerationInfo.h"

class ProceduralGrassPass
{
public:
	ProceduralGrassPass();
	~ProceduralGrassPass();

	void DoPass(IFrameBuffer* geometryBuffer, GrassCluster& cluster, const TerrainGenerationInfo& terrainInfo, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view);

	static const std::string GRASS_SHADER_KEY;

	static const std::string GRASS_SHADER_KEY;

private:
	Shader* shader;
};