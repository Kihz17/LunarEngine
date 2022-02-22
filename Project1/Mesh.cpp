#include "Mesh.h"

#include <iostream>

Mesh::Mesh(const std::vector<IVertex*>& vertices, const std::vector<Face>& indices, AABB* boundingBox)
	: vertexArray(nullptr),
	vertexBuffer(nullptr),
	indexBuffer(nullptr),
	vertices(vertices),
	faces(indices),
	boundingBox(boundingBox)
{
	BufferLayout bufferLayout = {
		{ ShaderDataType::Float3, "vPosition" },
		{ ShaderDataType::Float3, "vNormal" },
		{ ShaderDataType::Float2, "vTextureCoordinates" }
	};

	this->vertexArray = new VertexArrayObject();

	uint32_t vertexBufferSize = (uint32_t)(this->vertices.size() * Vertex::Size());
	float* vertexBuffer = MeshUtils::ConvertVerticesToArray(this->vertices);

	this->vertexBuffer = new VertexBuffer(vertexBuffer, vertexBufferSize);
	this->vertexBuffer->SetLayout(bufferLayout);

	uint32_t indexBufferSize = (uint32_t)(this->faces.size() * sizeof(Face));
	uint32_t* indexBuffer = MeshUtils::ConvertIndicesToArray(this->faces);

	this->vertexArray->Bind();
	this->indexBuffer = new IndexBuffer(indexBuffer, indexBufferSize);

	this->vertexArray->AddVertexBuffer(this->vertexBuffer);
	this->vertexArray->SetIndexBuffer(this->indexBuffer);

	delete[] vertexBuffer;
	delete[] indexBuffer;
}

Mesh::~Mesh()
{
	delete vertexArray;
	delete vertexBuffer;
	delete indexBuffer;

	for (IVertex* v : vertices) delete v;

	delete boundingBox;
}