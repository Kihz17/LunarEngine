#pragma once

#include "Shader.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "IFrameBuffer.h"
#include "TerrainGenerationInfo.h"

class TerrainPass
{
public:
	TerrainPass();

	void DoPass(IFrameBuffer* geometryBuffer, TerrainGenerationInfo& terrainInfo, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos);

	static const std::string TERRAIN_SHADER_KEY;

private:
	Texture2D* heightMap;
	VertexArrayObject* vao;
	VertexBuffer* vbo;
	int patchCount;
	int numPatchPrimitives;

	Shader* shader;

	Texture2D* terrainTexture;
	float terrainTextureScale;
};