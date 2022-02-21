#pragma once

#include "IMesh.h"
#include "AnimatedVertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class AnimatedMesh : public IMesh
{
public:
	AnimatedMesh(const std::string& path);
	virtual ~AnimatedMesh();

	virtual const std::vector<Submesh>& GetSubmeshes() const override { return this->submeshes; }

	virtual VertexArrayObject* GetVertexArray() override { return this->vertexArray; }
	virtual VertexBuffer* GetVertexBuffer() override { return this->vertexBuffer; }
	virtual IndexBuffer* GetIndexBuffer() override { return this->indexBuffer; }

	virtual const BufferLayout& GetVertexBufferLayout() const override { return this->vertexBuffer->GetLayout(); }

	virtual const std::vector<IVertex*>& GetVertices() const override { return this->vertices; }
	virtual const std::vector<Face>& GetFaces() const override { return this->faces; }

	virtual const AABB* GetBoundingBox() const override { return this->boundingBox; }

	virtual const std::string& GetPath() const override { return this->filePath; }

private:
	void SetupMaterials();
	void LoadNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f));
	
	Scope<Assimp::Importer> importer;

	std::vector<Submesh> submeshes;

	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	std::vector<IVertex*> vertices;
	std::vector<Face> faces;

	std::unordered_map<aiNode*, std::vector<uint32_t>> nodeMap;
	const aiScene* assimpScene;

	AABB* boundingBox;

	std::map<std::string, BoneInfo> boneInfo;
	int boneCount;

	std::string filePath;
};