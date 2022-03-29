#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"
#include "GrassCluster.h"

class GrassPass
{
public:
	GrassPass();
	~GrassPass();

	void DoPass(IFrameBuffer* geometryBuffer, std::vector<GrassCluster>& grassClusters, const glm::vec3& cameraPos, const glm::mat4& proj, const glm::mat4& view);

	static const std::string GRASS_SHADER_KEY;

private:
	Shader* shader;
};