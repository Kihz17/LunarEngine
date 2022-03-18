#pragma once

#include "Camera.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "GLCommon.h"

#include <glm/glm.hpp>

enum class ShapeType
{
	Cube,
	Plane,
	Quad
};

class PrimitiveShape
{
public:
	PrimitiveShape(ShapeType shapeType);
	virtual ~PrimitiveShape();

	void Draw();

	ShapeType GetShapeType() const { return type; }

	VertexArrayObject* GetVertexArray() const { return vertexArray; }
	int GetIndexCount() const { return type == ShapeType::Cube ? 36 : type == ShapeType::Plane ? 6 : type == ShapeType::Quad ? 4 : 0; }

private:
	ShapeType type;

	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
};