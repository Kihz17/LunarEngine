#pragma once

#include <vector>
#include <string>

#include<glm/glm.hpp>

struct Bone
{
	unsigned int ID = -1;
	std::string name;
	glm::mat4 offsetTransform = glm::mat4(1.0f); // The bones offset 
	glm::mat4 bindTransform = glm::mat4(1.0f); // The bones offset with parent transform applied to it

	std::vector<Bone> children;
};