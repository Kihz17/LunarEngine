#pragma once

#include <glm/glm.hpp>

class IVertex
{
public:
	virtual ~IVertex() {}

	virtual float* Data() = 0;
};