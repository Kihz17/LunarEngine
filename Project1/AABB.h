#pragma once

#include "IBoundingVolume.h"
#include "VertexArrayObject.h"
#include "VertexBuffer.h";
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <glm/glm.hpp>

class AABB : public IBoundingVolume
{
public:
	AABB();
	AABB(const glm::vec3& min, const glm::vec3 max);
	AABB(const glm::vec3& center, float sizeX, float sizeY, float sizeZ);
	virtual ~AABB();

	virtual bool IsOnFrustum(const Frustum& frustum, const glm::mat4& transform) const override;
	virtual bool IsOnOrForwardPlan(const Plan& plan) const override;

	void Resize(const glm::vec3& min, const glm::vec3 max);

	const glm::vec3& GetCenter() const { return center; }
	const glm::vec3& GetSize() const { return size; }

	glm::vec3 GetMin() const;
	glm::vec3 GetMax() const;

	VertexArrayObject* GetVertexArray() const { return vao; }
	unsigned int GetIndexCount() const { return 24; }
	
private:
	void SetupVertices();

	glm::vec3 center;
	glm::vec3 size;

	VertexArrayObject* vao;
	VertexBuffer* vbo;
	IndexBuffer* ebo;
};