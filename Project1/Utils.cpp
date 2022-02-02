#include "Utils.h"

#include <random>

static std::random_device random;
static std::mt19937 rng(random());

float Utils::RandFloat(float min, float max)
{
	std::uniform_real_distribution<float> distr(min, max);
	return distr(rng);
}
