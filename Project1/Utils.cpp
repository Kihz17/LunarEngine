#include "Utils.h"

#include <random>

static std::random_device random;
static std::mt19937 rng(random());

float Utils::RandFloat(float min, float max)
{
	std::uniform_real_distribution<float> distr(min, max);
	return distr(rng);
}

int Utils::RandInt(int min, int max)
{
	std::uniform_int_distribution<int> distr(min, max);
	return distr(rng);
}

bool Utils::RandBool()
{
	std::uniform_int_distribution<int> distr(0, 1);
	return distr(rng);
}
