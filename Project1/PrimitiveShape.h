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
private:
	ShapeType type;

	Ref<VertexArrayObject> vertexArray;
	Ref<VertexBuffer> vertexBuffer;
};