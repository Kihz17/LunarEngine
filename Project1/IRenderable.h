#pragma once

#include <glm/glm.hpp>

class IRenderable
{
public:
	virtual glm::vec3* GetRenderPosition() = 0;
	virtual glm::vec3* GetRenderScale() = 0;
	virtual glm::vec3* GetRenderOrientation() = 0;
};