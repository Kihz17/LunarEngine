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
	Mesh(const std::vector<IVertex*>& vertices, const std::vector<Face>& indices, AABB* boundingBox);
	virtual ~Mesh();

	virtual VertexArrayObject* GetVertexArray() override { return this->vertexArray; }
	virtual VertexBuffer* GetVertexBuffer() override { return this->vertexBuffer; }
	virtual IndexBuffer* GetIndexBuffer() override { return this->indexBuffer; }

	virtual const BufferLayout& GetVertexBufferLayout() const override { return this->vertexBuffer->GetLayout(); }

	virtual const std::vector<IVertex*>& GetVertices() const override { return this->vertices; }
	virtual const std::vector<Face>& GetFaces() const override { return this->faces; }

	virtual const AABB* GetBoundingBox() const override { return this->boundingBox; }

private:
	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	std::vector<IVertex*> vertices;
	std::vector<Face> faces;

	AABB* boundingBox;
};