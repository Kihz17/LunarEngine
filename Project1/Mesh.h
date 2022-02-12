#pragma once

#include "pch.h"
#include "AABB.h"
#include "VertexInformation.h"
#include "VertexArrayObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>

struct Submesh
{
	Submesh() : boundingBox(new AABB()) {}

	uint32_t baseVertex;
	uint32_t baseIndex;
	uint32_t materialIndex;
	uint32_t indexCount;
	uint32_t vertexCount;

	AABB* boundingBox;

	glm::mat4 transform{ 1.0f };
	std::string nodeName;
	std::string meshName;
};

class Mesh
{
public:
	Mesh(const std::string& filePath);
	Mesh(const Ref<Mesh> mesh);
	virtual ~Mesh();
	
	inline std::vector<Submesh>& GetSubmeshes() { return this->submeshes; }
	inline const std::vector<Submesh>& GetSubmeshes() const { return this->submeshes; }

	inline VertexArrayObject* GetVertexArray() { return this->vertexArray; }
	inline VertexBuffer* GetVertexBuffer() { return this->vertexBuffer; }
	inline IndexBuffer* GetIndexBuffer() { return this->indexBuffer; }

	const BufferLayout& GetVertexBufferLayout() const { return this->vertexBuffer->GetLayout(); }

	inline const std::vector<Vertex>& GetVertices() const { return this->vertices; }
	inline const std::vector<Face>& GetFaces() const { return this->faces; }

	inline const AABB* GetBoundingBox() const { return this->boundingBox; }

	inline const std::string& GetPath() const { return this->filePath; }

private:
	void SetupMaterials();
	void LoadNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f));

	Scope<Assimp::Importer> importer;

	std::vector<Submesh> submeshes;
	glm::mat4 inverseTransform;

	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<Face> faces;

	std::unordered_map<aiNode*, std::vector<uint32_t>> nodeMap;
	const aiScene* assimpScene;

	AABB* boundingBox;

	std::string filePath;
};