#pragma once

#include "IVertex.h"
#include "VertexArrayObject.h"
#include "AABB.h"

struct Submesh
{
	Submesh() 
		: baseVertex(0),
		baseIndex(0),
		materialIndex(0),
		indexCount(0),
		vertexCount(0),
		boundingBox(new AABB()) ,
		transform(1.0f)
	{}

	uint32_t baseVertex;
	uint32_t baseIndex;
	uint32_t materialIndex;
	uint32_t indexCount;
	uint32_t vertexCount;

	AABB* boundingBox;

	glm::mat4 transform;
	std::string nodeName;
	std::string meshName;
};

class IMesh
{
public:
	virtual ~IMesh() {}

	virtual const std::vector<Submesh>& GetSubmeshes() const = 0;

	virtual VertexArrayObject* GetVertexArray() = 0;
	virtual VertexBuffer* GetVertexBuffer() = 0;
	virtual IndexBuffer* GetIndexBuffer() = 0;
	virtual const BufferLayout& GetVertexBufferLayout() const = 0;

	virtual const std::vector<IVertex*>& GetVertices() const = 0;
	virtual const std::vector<Face>& GetFaces() const = 0;

	virtual const AABB* GetBoundingBox() const = 0;

	virtual const std::string& GetPath() const = 0;
};

class MeshUtils
{
public:

	static const uint32_t ASSIMP_FLAGS;

	static glm::mat4 ConvertToGLMMat4(const aiMatrix4x4& matrix);

	static float* ConvertVerticesToArray(const std::vector<IVertex*>& vertices);
	static float* ConvertAnimatedVerticesToArray(const std::vector<IVertex*>& vertices);

	static uint32_t* ConvertIndicesToArray(const std::vector<Face>& faces);
};