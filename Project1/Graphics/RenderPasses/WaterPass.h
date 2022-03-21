#pragma once

#include "Texture2D.h"

#include <glm/glm.hpp>

class WaterPass
{
public:

	static Texture2D* GenH0();
	static Texture2D* GenW();
	static float Phillips(const glm::vec2& pos, const glm::vec2& windDir, float windSpeed, float A, float gravityAccel);
};