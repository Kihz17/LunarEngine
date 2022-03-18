#pragma once

#include "IVertex.h"

class Vertex : public IVertex
{
public:
	Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord);
	virtual ~Vertex();

	virtual float* Data() override { return data; }

	static unsigned int Length() { return 8; }
	static unsigned int Size() { return sizeof(float) * 8; }

private:
	float data[8];
};