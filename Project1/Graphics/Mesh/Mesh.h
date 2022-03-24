#pragma once

#include "IMesh.h"
#include "Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>

class Mesh : public IMesh
{
public:
	Mesh(const std::string& filePath);
	virtual ~Mesh();

	virtual std::vector<Submesh>& GetSubmeshes() override { return submeshes; }

	virtual VertexArrayObject* GetVertexArray() override { return this->vertexArray; }
	virtual VertexBuffer* GetVertexBuffer() override { return this->vertexBuffer; }
	virtual IndexBuffer* GetIndexBuffer() override { return this->indexBuffer; }

	virtual const BufferLayout& GetVertexBufferLayout() const override { return this->vertexBuffer->GetLayout(); }

	virtual const std::vector<IVertex*>& GetVertices() const override { return this->vertices; }
	virtual const std::vector<Face>& GetFaces() const override { return this->indices; }

	virtual AABB* GetBoundingBox() const override { return this->boundingBox; }

private:
	void ParseMesh(unsigned int meshIndex, const aiMesh* assimpMesh);
	void ParseMaterials(const aiScene* assimpScene);
	void ParseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f));

	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	std::vector<IVertex*> vertices;
	std::vector<Face> indices;

	std::vector<Submesh> submeshes;
	AABB* boundingBox;
	std::string filePath;
};