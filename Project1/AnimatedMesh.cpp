#include "AnimatedMesh.h"

#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

#include <iostream>

AnimatedMesh::AnimatedMesh(const std::vector<IVertex*>& vertices, const std::vector<Face>& indices, AABB* boundingBox)
	: vertexArray(nullptr),
	vertexBuffer(nullptr),
	indexBuffer(nullptr),
	vertices(vertices),
	faces(indices),
	boundingBox(boundingBox),
	boneCount(0)
{
	BufferLayout bufferLayout;
	if (MAX_BONE_INFLUENCE <= 4)
	{
		bufferLayout  = {
			{ ShaderDataType::Float3, "vPosition" },
			{ ShaderDataType::Float3, "vNormal" },
			{ ShaderDataType::Float2, "vTextureCoordinates" },
			{ ShaderDataType::Int4, "vBoneIDs" },
			{ ShaderDataType::Float4, "vBoneWeights" }
		};
	}
	else
	{
		std::cout << "Max bone influence was greater than 8!\n";
		return;
	}

	this->vertexArray = new VertexArrayObject();

	uint32_t vertexBufferSize = (uint32_t)(this->vertices.size() * AnimatedVertex::Size());
	float* vertexBuffer = MeshUtils::ConvertAnimatedVerticesToArray(this->vertices);

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

AnimatedMesh::~AnimatedMesh()
{
	delete vertexArray;
	delete vertexBuffer;
	delete indexBuffer;

	for (IVertex* v : vertices) delete v;

	delete boundingBox;
}