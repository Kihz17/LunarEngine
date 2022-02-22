#pragma once

#include "IVertex.h"
#include "VertexArrayObject.h"
#include "AABB.h"

class IMesh
{
public:
	virtual ~IMesh() {}

	virtual VertexArrayObject* GetVertexArray() = 0;
	virtual VertexBuffer* GetVertexBuffer() = 0;
	virtual IndexBuffer* GetIndexBuffer() = 0;
	virtual const BufferLayout& GetVertexBufferLayout() const = 0;

	virtual const std::vector<IVertex*>& GetVertices() const = 0;
	virtual const std::vector<Face>& GetFaces() const = 0;

	virtual const AABB* GetBoundingBox() const = 0;
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