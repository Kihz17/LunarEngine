#include "Vertex.h"

Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord)
{
	data[0] = position.x;
	data[1] = position.y;
	data[2] = position.z;

	data[3] = normal.x;
	data[4] = normal.y;
	data[5] = normal.z;

	data[6] = texCoord.x;
	data[7] = texCoord.y;
}

Vertex::~Vertex()
{

}